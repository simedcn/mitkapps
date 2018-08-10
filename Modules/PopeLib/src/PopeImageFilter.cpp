/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <PopeImageFilter.h>

#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>

#include <itkShiftScaleImageFilter.h>
#include <itkImageMomentsCalculator.h>
#include <itkTranslationTransform.h>
#include <itkResampleImageFilter.h>

#include <memory>
#include <vector>
#include <assert.h>

using namespace std;


template<typename TPixel, unsigned int VImageDimension>
static void GetCenter(const itk::Image<TPixel, VImageDimension>* inputImage, shared_ptr<vector<double>> center)
{
	using ImageType = itk::Image<TPixel, VImageDimension>;
	using FixedImageCalculatorType = itk::ImageMomentsCalculator<ImageType>;
	FixedImageCalculatorType::Pointer fixedCalculator = FixedImageCalculatorType::New();
	fixedCalculator->SetImage(inputImage);
	fixedCalculator->Compute();
	FixedImageCalculatorType::VectorType fixedCenter = fixedCalculator->GetCenterOfGravity();
	center->resize(fixedCenter.GetVectorDimension());
	for (size_t i = 0; i < fixedCenter.GetVectorDimension(); i++)
	{
		center->operator[](i) = static_cast<double>(fixedCenter[i]);
	}
}

template<typename TPixel, unsigned int VImageDimension>
static void Translate(const itk::Image<TPixel, VImageDimension>* inputImage, shared_ptr<vector<double>> center, mitk::Image::Pointer outputImage)
{
	using ImageType = itk::Image<TPixel, VImageDimension>;
	using FilterType = itk::ShiftScaleImageFilter<ImageType, ImageType>;
	using FixedImageCalculatorType = itk::ImageMomentsCalculator<ImageType>;

	FixedImageCalculatorType::Pointer calculator = FixedImageCalculatorType::New();
	calculator->SetImage(inputImage);
	calculator->Compute();
	FixedImageCalculatorType::VectorType imageCenter = calculator->GetCenterOfGravity();

	//const unsigned int dimension = VImageDimension; // 3; // imageCenter.GetVectorDimension();
	assert(VImageDimension == imageCenter.GetVectorDimension());
	using TranslationTransformType = itk::TranslationTransform<double, VImageDimension>;
	TranslationTransformType::Pointer transform = TranslationTransformType::New();
	TranslationTransformType::OutputVectorType translation;
	for (size_t i = 0; i < VImageDimension; i++)
	{
		translation[i] = imageCenter[i] - center->operator[](i);
	}
	transform->Translate(translation);
	MITK_INFO << "Translation: " << translation;

	using ResampleImageFilterType = itk::ResampleImageFilter<ImageType, ImageType>;
	ResampleImageFilterType::Pointer resampleFilter = ResampleImageFilterType::New();
	resampleFilter->SetTransform(transform.GetPointer());
	resampleFilter->SetInput(inputImage);
	// Without this, the program crashes
	ImageType::SizeType size = inputImage->GetLargestPossibleRegion().GetSize();
	resampleFilter->SetSize(size);
	resampleFilter->Update();

	// This is the tricky part that is done wrong very often. As the image data
	// of ITK images and MITK images are binary compatible, we don't need to
	// cast or copy the ITK output image. Instead, we just want to reference
	// the image data and tell ITK that we took the ownership.
	mitk::GrabItkImageMemory(resampleFilter->GetOutput(), outputImage);
}

// See definition of PopeImageFilter::GenerateData() further below for a rationale behind this function template.
template<typename TPixel, unsigned int VImageDimension>
static void AddOffset(const itk::Image<TPixel, VImageDimension>* inputImage, int offset, mitk::Image::Pointer outputImage)
{
	typedef itk::Image<TPixel, VImageDimension> ImageType;
	typedef itk::ShiftScaleImageFilter<ImageType, ImageType> FilterType;

	auto filter = FilterType::New();
	filter->SetInput(inputImage);
	filter->SetShift(offset);
	filter->Update();

	// This is the tricky part that is done wrong very often. As the image data
	// of ITK images and MITK images are binary compatible, we don't need to
	// cast or copy the ITK output image. Instead, we just want to reference
	// the image data and tell ITK that we took the ownership.
	mitk::GrabItkImageMemory(filter->GetOutput(), outputImage);
}

PopeImageFilter::PopeImageFilter()
	: m_Offset(0)
{
	this->SetNumberOfRequiredInputs(2);
	this->SetNumberOfRequiredOutputs(1);
}

PopeImageFilter::~PopeImageFilter()
{
}

void PopeImageFilter::GenerateData()
{
	mitk::Image::Pointer inputImage = this->GetInput(0);
	mitk::Image::Pointer fixedImage = this->GetInput(1);

	mitk::Image::Pointer translatedImage;
	if (fixedImage == nullptr)
	{
		translatedImage = inputImage;
	}
	else
	{
		translatedImage = this->GetOutput();
		try
		{
			// We want to apply an ITK filter to the MITK input image. While MITK
			// images are not templated, ITK images are templated by both pixel type
			// and image dimension. The actual image data is binary compatible, though.
			// MITK provides ITK access macros that enable you to directly operate
			// on MITK images without any superfluous copying.
			// To allow ITK filters to work with different image types at runtime you
			// would be required to instantiate your function templates for each and
			// every expected combination of pixel type and image dimension. Luckily,
			// MITK provides a whole bunch of multiplexer macros to save you doing this
			// manually (see mitkImageAccessByItk.h).
			// These macros range from being completely generic to partly constrained
			// variants. For example, you may want to constrain the image dimension or
			// the pixel type. As your function template is compiled for each allowed
			// combination, compile time and code size may increase dramatically.
			// As a rule of thumb, use a suitable multiplexer macro that is as
			// constrained as possible and yet as generic as necessary.
			// To prevent a combinatorial explosion, image dimension is restricted to
			// 2 and 3 even for the dimension-variable multiplexer macros.
			// Thus, the following multiplexer macro allows for 2-dimensional and
			// 3-dimensional images with an integer pixel type, for example,
			// (un)signed char, short, and int, resulting in a total of 12 distinct
			// combinations.
			auto center = make_shared<vector<double>>();
			AccessIntegralPixelTypeByItk_n(fixedImage, GetCenter, (center));
			AccessIntegralPixelTypeByItk_n(inputImage, Translate, (center, translatedImage));
		}
		catch (const mitk::AccessByItkException& e)
		{
			MITK_ERROR << "Unsupported pixel type or image dimension: " << e.what();
		}
	}

	if (m_Offset == 0)
	{
		// Nothing to calculate in this case, just copy the input image.
		this->SetPrimaryOutput(translatedImage->Clone().GetPointer());
	}
	else
	{
		mitk::Image::Pointer outputImage = translatedImage;

		try
		{
			// We want to apply an ITK filter to the MITK input image. While MITK
			// images are not templated, ITK images are templated by both pixel type
			// and image dimension. The actual image data is binary compatible, though.
			// MITK provides ITK access macros that enable you to directly operate
			// on MITK images without any superfluous copying.
			// To allow ITK filters to work with different image types at runtime you
			// would be required to instantiate your function templates for each and
			// every expected combination of pixel type and image dimension. Luckily,
			// MITK provides a whole bunch of multiplexer macros to save you doing this
			// manually (see mitkImageAccessByItk.h).
			// These macros range from being completely generic to partly constrained
			// variants. For example, you may want to constrain the image dimension or
			// the pixel type. As your function template is compiled for each allowed
			// combination, compile time and code size may increase dramatically.
			// As a rule of thumb, use a suitable multiplexer macro that is as
			// constrained as possible and yet as generic as necessary.
			// To prevent a combinatorial explosion, image dimension is restricted to
			// 2 and 3 even for the dimension-variable multiplexer macros.
			// Thus, the following multiplexer macro allows for 2-dimensional and
			// 3-dimensional images with an integer pixel type, for example,
			// (un)signed char, short, and int, resulting in a total of 12 distinct
			// combinations.
			AccessIntegralPixelTypeByItk_n(inputImage, AddOffset, (m_Offset, outputImage));
		}
		catch (const mitk::AccessByItkException& e)
		{
			MITK_ERROR << "Unsupported pixel type or image dimension: " << e.what();
		}
	}
}

/*void PopeImageFilter::SetFixedImage(mitk::Image* image)
{
	m_FixedImage = image;
}*/