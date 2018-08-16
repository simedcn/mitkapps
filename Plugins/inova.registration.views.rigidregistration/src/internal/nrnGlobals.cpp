#include "nrnGlobals.h"

#include "itkShrinkImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"


//#include "itkGPUImage.h"
//#include "itkGPUKernelManager.h"
//#include "itkGPUContextManager.h"
//#include "itkGPUImageToImageFilter.h"
//#include "itkGPUDiscreteGaussianImageFilter.h"
#include <iomanip>

#include "itkMersenneTwisterRandomVariateGenerator.h"

template<typename ImageType>
typename ImageType::Pointer nrnLoadImage(std::string filename)
{
	typedef itk::ImageFileReader<ImageType> ReaderType;
	typename ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(filename);
	reader->Update();
	return reader->GetOutput();
}

template<typename ImageType>
void nrnSaveImage(std::string filename, typename ImageType::Pointer image)
{
	typedef  itk::ImageFileWriter< ImageType  > WriterType;
	typename WriterType::Pointer writer = WriterType::New();
	writer->SetFileName(filename);
	writer->SetInput(image);
	writer->Update();
}

template<typename ImageType, typename TransformType>
typename ImageType::Pointer resampleImage(typename ImageType::Pointer image,typename ImageType::Pointer referenceImage)
{
	typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleImageFilterType;
	typename ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
	resample->SetInput(image);
	resample->SetOutputParametersFromImage(referenceImage);
	resample->SetTransform(TransformType::New());
	resample->UpdateLargestPossibleRegion();
	return resample->GetOutput();
}

template<typename ImageType, typename TransformType>
typename ImageType::Pointer resampleImage(typename ImageType::Pointer image,
										  typename ImageType::Pointer referenceImage,
										  typename TransformType::Pointer transform	)
{
	typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleImageFilterType;
	typename ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
	resample->SetInput(image);
	resample->SetOutputParametersFromImage(referenceImage);
	resample->SetTransform(transform);
	resample->UpdateLargestPossibleRegion();
	resample->SetDefaultPixelValue(0);
	return resample->GetOutput();
}


template<typename PointSetType>
typename PointSetType::Pointer nrnLoadLandmarks(std::string filename)
{
	typename PointSetType::Pointer fixedPointSet  = PointSetType::New();
	typedef typename PointSetType::PointType     PointType;
	typedef typename PointSetType::PointsContainer  PointsContainer;
	typename PointsContainer::Pointer fixedPointContainer  = PointsContainer::New();
	PointType fixedPoint;

	std::ifstream   fixedFile;
	fixedFile.open( filename.c_str() );
	if( fixedFile.fail() )
	{
		return fixedPointSet;
	}

	unsigned int pointId = 0;
	
	while( !fixedFile.eof() )
	{
		fixedFile >> fixedPoint;
		fixedPointContainer->InsertElement( pointId, fixedPoint );
		pointId++;
	}
	fixedPointSet->SetPoints( fixedPointContainer );
	return fixedPointSet;
}

template<typename PointSetType>
void nrnSaveLandmarks(typename PointSetType::Pointer landmarks, std::string filename)
{
	std::fstream fs;
	fs.open ((char*)filename, std::fstream::out /*| std::fstream::app*/);

	fs << landmarks->GetPoint(0)[0] << " " 
			<< landmarks->GetPoint(0)[1] << " "
			<< landmarks->GetPoint(0)[2] ;
	for (unsigned int i=1; i < landmarks->GetNumberOfPoints(); ++i)	
		fs << "\n"<<landmarks->GetPoint(i)[0] << " " 
			<< landmarks->GetPoint(i)[1] << " "
			<< landmarks->GetPoint(i)[2];
	
	fs.close();
}

template<typename ImageType, typename TransformType>
void nrnSaveTransformationFile(typename ImageType::Pointer fixedImage, 
								typename TransformType::Pointer transform,
								std::string filename)
{
	typename ImageType::PointType pi[8];
	typename ImageType::PointType po[8];
	typename ImageType::IndexType u; 
	typename ImageType::SizeType size = fixedImage->GetLargestPossibleRegion().GetSize();
	u.Fill(0);
		fixedImage->TransformIndexToPhysicalPoint(u,pi[0]);
	u[0] = size[0]-1;
		fixedImage->TransformIndexToPhysicalPoint(u,pi[1]);
	u[0] = 0; u[1] = size[1]-1; 
		fixedImage->TransformIndexToPhysicalPoint(u,pi[2]);
	u[0] =  size[0]-1; 
		fixedImage->TransformIndexToPhysicalPoint(u,pi[3]);
	u[0] = 0; u[1] = 0; u[2] = size[2]-1; 
		fixedImage->TransformIndexToPhysicalPoint(u,pi[4]);
	u[0] = size[0]-1;
		fixedImage->TransformIndexToPhysicalPoint(u,pi[5]);
	u[0] = 0; u[1] = size[1]-1; 
		fixedImage->TransformIndexToPhysicalPoint(u,pi[6]);
	u[0] =  size[0]-1; 
		fixedImage->TransformIndexToPhysicalPoint(u,pi[7]);
	for (unsigned int i=0; i < 8; ++i)
		po[i] = transform->TransformPoint(pi[i]);
	
	
	std::fstream fs;
	fs.open (filename.c_str(), std::fstream::out /*| std::fstream::app*/);

	fs << "-------------------------------------------------------------------------\n"
	   << "Transformation Parameters\n\n"
	   << "Investigator(s): D. Glodeck\n\n"
	   << "Site:\n\n"
	   << "Method: " << "Testmethod 1\n"
	   << "Date: " << "11.05.2016\n"
	   << "Patient number " << "001" <<"\n"
	   << "From: " << "CT" <<"\n"
	   << "To: "<< "mr_T2" << "\n\n"
	   << "Point      x          y          z        new_x       new_y       new_z\n\n";
	
	fs.precision(4);
	for (unsigned int i=0; i < 8; ++i)
		fs << "  " << i+1 << "  " <<    std::setw(10) 
					<< pi[i][0] << " " << std::setw(10)<< pi[i][1] << " " << std::setw(10)<< pi[i][2] << " " 
					<< std::setw(10) << po[i][0] << " " << std::setw(11)<<  po[i][1] << " " <<std::setw(11)<< po[i][2] << "\n";
	

	fs << "\n(All distances are in millimeters.)\n"
	   << "-------------------------------------------------------------------------\n";

	fs.close();
}

template<typename PointSetType>
void nrnPrintLandmarks(typename PointSetType::Pointer landmarks)
{
	for (unsigned int i=0; i < landmarks->GetNumberOfPoints(); ++i)
			std::cout << landmarks->GetPoint(i)<<"\n";
}

template<typename PointSetType>
typename PointSetType::Pointer displaceLandmarks(typename PointSetType::Pointer landmarks, 
												typename PointSetType::PointType::VectorType displacement)
{
	typename PointSetType::Pointer newPointSet  = PointSetType::New();
	typedef typename PointSetType::PointType     PointType;
	typedef typename PointSetType::PointsContainer  PointsContainer;
	typename PointsContainer::Pointer pointContainer  = PointsContainer::New();
	PointType point;


	
	for (unsigned int i=0; i < landmarks->GetNumberOfPoints(); ++i)
	{
		point = landmarks->GetPoint(i) + displacement;
		pointContainer->InsertElement( i, point );

	}
	newPointSet->SetPoints( pointContainer );
	return newPointSet;

}

template<typename PointSetType,typename ImageType>
typename PointSetType::Pointer createReferenceLandmarks( typename ImageType::Pointer referenceImage)
{
	typename PointSetType::Pointer newPointSet  = PointSetType::New();
	typedef typename PointSetType::PointType     PointType;
	typedef typename PointSetType::PointsContainer  PointsContainer;
	typename PointsContainer::Pointer pointContainer  = PointsContainer::New();
	PointType point;

  // compute maximal region:
	double xRange = referenceImage->GetSpacing()[0]*referenceImage->GetLargestPossibleRegion().GetSize()[0];
	double yRange = referenceImage->GetSpacing()[1]*referenceImage->GetLargestPossibleRegion().GetSize()[1];
	double zRange = referenceImage->GetSpacing()[2]*referenceImage->GetLargestPossibleRegion().GetSize()[2];
  
	PointType origin = referenceImage->GetOrigin();
	//std::cout << "origin: " << origin<<"\n";

	typename ImageType::DirectionType direction = referenceImage->GetDirection();
	//std::cout << "direction: " << direction<<"\n";*/

  // define new points:

	point[0] = origin[0]+ direction[0][0] * xRange/4;
	point[1] = origin[1]+ direction[1][1] * yRange/4;
	point[2] = origin[2]+ direction[2][2] * zRange/4;
	pointContainer->InsertElement( 0, point );
	point[0] = origin[0]+ direction[0][0] * 3.0*xRange/4;
	point[1] = origin[1]+ direction[1][1] * yRange/4;
	point[2] = origin[2]+ direction[2][2] * zRange/4;
	pointContainer->InsertElement( 1, point );
	point[0] = origin[0]+ direction[0][0] * xRange/4;
	point[1] = origin[1]+ direction[1][1] * 3.0*yRange/4;
	point[2] = origin[2]+ direction[2][2] * zRange/4;
	pointContainer->InsertElement( 2, point );
	point[0] = origin[0]+ direction[0][0] * xRange/4;
	point[1] = origin[1]+ direction[1][1] * yRange/4;
	point[2] = origin[2]+ direction[2][2] * 3.0*zRange/4;
	pointContainer->InsertElement( 3, point );
	point[0] = origin[0]+ direction[0][0] * 3.0*xRange/4;
	point[1] = origin[1]+ direction[1][1] * 3.0*yRange/4;
	point[2] = origin[2]+ direction[2][2] * zRange/4;
	pointContainer->InsertElement( 4, point );
	point[0] = origin[0]+ direction[0][0] * 3.0*xRange/4;
	point[1] = origin[1]+ direction[1][1] * yRange/4;
	point[2] = origin[2]+ direction[2][2] * 3.0*zRange/4;
	pointContainer->InsertElement( 5, point );
	point[0] = origin[0]+ direction[0][0] * xRange/4;
	point[1] = origin[1]+ direction[1][1] * 3.0*yRange/4;
	point[2] = origin[2]+ direction[2][2] * 3.0*zRange/4;
	pointContainer->InsertElement( 6, point );
	point[0] = origin[0]+ direction[0][0] * 3.0*xRange/4;
	point[1] = origin[1]+ direction[1][1] * 3.0*yRange/4;
	point[2] = origin[2]+ direction[2][2] * 3.0*zRange/4;
	pointContainer->InsertElement( 7, point );
	
	newPointSet->SetPoints( pointContainer );

	return newPointSet;
}

template<typename PointSetType,typename TransformType>
typename PointSetType::Pointer createMovingLandmarks( typename PointSetType::Pointer inputLandmarks, 
													  typename TransformType::Pointer transform,
													  typename PointSetType::PointType::VectorType displacement)
{
	typename PointSetType::Pointer newPointSet  = PointSetType::New();
	typedef typename PointSetType::PointType     PointType;
	typedef typename PointSetType::PointsContainer  PointsContainer;
	typename PointsContainer::Pointer pointContainer  = PointsContainer::New();
	PointType point;
	for (unsigned int i=0; i < inputLandmarks->GetNumberOfPoints(); ++i)
	{
		point= static_cast<PointType>(transform->TransformPoint(inputLandmarks->GetPoint(i)))+ displacement;
		pointContainer->InsertElement( i, point );
	}
	newPointSet->SetPoints( pointContainer );
	return newPointSet;
}



template<typename ImageType>
typename ImageType::Pointer OtsuFilter(typename ImageType::Pointer image)
{
	typename ImageType::Pointer output;
	typedef typename itk::OtsuThresholdImageFilter<
			ImageType, ImageType >  FilterType;
	typename FilterType::Pointer filter = FilterType::New();
	filter->SetInput(image);
	filter->SetOutsideValue(1);
	filter->SetInsideValue(0);
	filter->Update();
	//std::cout << "Otsu filter threshold: "<<filter->GetThreshold()<<"\n";
	output = filter->GetOutput();
	output->Update();
	output->DisconnectPipeline();
	return output;
}

template<typename ImageType>
typename ImageType::Pointer GradientImageFilter(typename ImageType::Pointer image)
{
	typename ImageType::Pointer output;
	typedef typename itk::GradientMagnitudeRecursiveGaussianImageFilter<
						 ImageType,ImageType >  filterType;
	typename filterType::Pointer gradientFilter = filterType::New();
	gradientFilter->SetInput( image );
	gradientFilter->Update();
	output = gradientFilter->GetOutput();
	output->Update();
	output->DisconnectPipeline();
	return output;
}

template<typename ImageType>
typename ImageType::Pointer smoothImage(typename ImageType::Pointer image, double sigma)
{
	typename ImageType::Pointer outputImage;
	typedef typename itk::DiscreteGaussianImageFilter<ImageType, ImageType> ImageSmoothingFilterType;
	typename ImageSmoothingFilterType::Pointer imageSmoothingFilter = ImageSmoothingFilterType::New();
	imageSmoothingFilter->SetUseImageSpacingOn();
	imageSmoothingFilter->SetVariance( vnl_math_sqr( sigma ) );
	imageSmoothingFilter->SetMaximumError( 0.01 );
	imageSmoothingFilter->SetInput( image );
	outputImage =imageSmoothingFilter->GetOutput();
	outputImage->Update();
	outputImage->DisconnectPipeline();
	return outputImage;
}


template<typename ImageType>
typename ImageType::Pointer smooth2Image(typename ImageType::Pointer image, double sigma)
{
	typename ImageType::Pointer outputImage;
	typedef typename itk::RecursiveGaussianImageFilter<ImageType, ImageType> FilterType;

	typename FilterType::Pointer filterX = FilterType::New();  
	typename FilterType::Pointer filterY = FilterType::New();
	//FilterType::Pointer filterZ = FilterType::New();
	filterX->SetDirection( 0 );   
	filterY->SetDirection( 1 );
	//filterZ->SetDirection( 2 );

	filterX->SetOrder( FilterType::ZeroOrder );  
	filterY->SetOrder( FilterType::ZeroOrder );
    //filterZ->SetOrder( FilterType::ZeroOrder );

	filterX->SetInput( image );  
	filterY->SetInput( filterX->GetOutput() );
	//filterZ->SetInput( filterY->GetOutput() );

	filterX->SetSigma( sigma );  
	filterY->SetSigma( sigma );
	//filterZ->SetSigma( sigma );

	outputImage =filterY->GetOutput();
	outputImage->Update();
	outputImage->DisconnectPipeline();
	return outputImage;
}


template<typename ImageType>
typename ImageType::Pointer smooth3DImage(typename ImageType::Pointer image, double sigma)
{
	typename ImageType::Pointer outputImage;
	typedef typename itk::RecursiveGaussianImageFilter<ImageType, ImageType> FilterType;

	typename FilterType::Pointer filterX = FilterType::New();  
	typename FilterType::Pointer filterY = FilterType::New();
	typename FilterType::Pointer filterZ = FilterType::New();
	filterX->SetDirection( 0 );   
	filterY->SetDirection( 1 );
	filterZ->SetDirection( 2 );

	filterX->SetOrder( FilterType::ZeroOrder );  
	filterY->SetOrder( FilterType::ZeroOrder );
    filterZ->SetOrder( FilterType::ZeroOrder );

	filterX->SetInput( image );  
	filterY->SetInput( filterX->GetOutput() );
	filterZ->SetInput( filterY->GetOutput() );

	filterX->SetSigma( sigma );  
	filterY->SetSigma( sigma );
	filterZ->SetSigma( sigma );

	outputImage =filterY->GetOutput();
	outputImage->Update();
	outputImage->DisconnectPipeline();
	return outputImage;
}


template<typename ImageType>
typename ImageType::Pointer smooth3Image(typename ImageType::Pointer image, double sigma)
{
/*	typename ImageType::Pointer outputImage;
	typedef typename itk::GPUDiscreteGaussianImageFilter< ImageType, ImageType> GPUFilterType;

	typename typename GPUFilterType::Pointer GPUFilter = GPUFilterType::New();

      itk::TimeProbe gputimer;
      gputimer.Start();

      GPUFilter->SetInput(image );
      GPUFilter->SetVariance( vnl_math_sqr( sigma ) );
      GPUFilter->Update();

      GPUFilter->GetOutput()->UpdateBuffers(); // synchronization point (GPU->CPU memcpy)

      gputimer.Stop();
      std::cout << "GPU Gaussian Filter took " << gputimer.GetMean() << " seconds.\n" << std::endl;

	outputImage =GPUFilter->GetOutput();
	outputImage->Update();
	outputImage->DisconnectPipeline();
	return outputImage;*/
}




template<typename ImageType>
typename ImageType::Pointer shrinkImage2(typename ImageType::Pointer image, double factor)
{
	typename ImageType::Pointer outputImage;
	if(factor < 1)
	{
		outputImage = image->Clone();
		return outputImage;
	}
	//std::cout << "input image:\n"<< image<< "\n";
	std::cout << "input Region\n"<<image->GetLargestPossibleRegion()<<"\n";
	typedef typename itk::IdentityTransform<double> TransformType;
	typedef typename itk::ResampleImageFilter<ImageType, ImageType> ResampleImageFilterType;
	typename ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
	//typedef itk::BSplineInterpolateImageFunction<ImageType, double, double> T_Interpolator;
	typedef typename itk::LinearInterpolateImageFunction<ImageType, double> T_Interpolator;
  // Resize
	typename T_Interpolator::Pointer _pInterpolator = T_Interpolator::New();
	//_pInterpolator->SetSplineOrder(3);

	const typename ImageType::SizeType inputSize = image->GetLargestPossibleRegion().GetSize();
	typename ImageType::SizeType outputSize;
	const typename ImageType::SpacingType inputSpacing = image->GetSpacing();
	typename ImageType::SpacingType outputSpacing;
	
	
	outputSpacing[0] = inputSpacing[0]*factor; 
	outputSpacing[1] = inputSpacing[1]*factor; 
	outputSpacing[2] = inputSpacing[2]*factor;
	typedef typename ImageType::SizeType::SizeValueType SizeValueType;
	outputSize[0] = static_cast<SizeValueType>((static_cast<double>(inputSize[0]) / factor) + .5);
	outputSize[1] = static_cast<SizeValueType>((static_cast<double>(inputSize[1]) / factor) + .5);
	outputSize[2] = static_cast<SizeValueType>((static_cast<double>(inputSize[2]) / factor) + .5);
	typename ImageType::PointType origin;
	origin = image->GetOrigin();
	//origin[2];
	resample->SetOutputOrigin ( origin);


	resample->SetInput(image);
	resample->SetInterpolator(_pInterpolator);
	resample->SetSize(outputSize);
	resample->SetOutputSpacing(outputSpacing);
 	resample->SetTransform(TransformType::New());
	resample->UpdateLargestPossibleRegion();
	
	
	outputImage= resample->GetOutput();
	outputImage->Update();
	outputImage->DisconnectPipeline();
	std::cout << "output Region\n"<<outputImage->GetLargestPossibleRegion()<<"\n";
	return outputImage;
}

template<typename ImageType>
typename ImageType::Pointer shrinkImage(typename ImageType::Pointer image, double factor)
{
	typename ImageType::Pointer outputImage;
	typedef typename itk::ShrinkImageFilter<ImageType, ImageType>         ShrinkFilterType;
	typename ShrinkFilterType::Pointer shrinkFilter = ShrinkFilterType::New();
	shrinkFilter->SetShrinkFactors( factor );
	shrinkFilter->SetInput( image );
	outputImage = shrinkFilter->GetOutput();
	outputImage->Update();
	outputImage->DisconnectPipeline();
	return outputImage;
}


template<typename ImageType>
typename ImageType::Pointer smoothAndScalePlane(typename ImageType::Pointer image, double factor)
{
	//TODO
}

/**
 * generate isotropic spacing acording to the in plane spacing (asume isotropic in plane spacing: using the x-spacing)
 */
template<typename ImageType>
typename ImageType::Pointer generateIsotropicSpacing(typename ImageType::Pointer image)
{
	typename ImageType::Pointer outputImage;
	//std::cout << "input image:\n"<< image<< "\n";

	typedef typename itk::IdentityTransform<double> TransformType;
	typedef typename itk::ResampleImageFilter<ImageType, ImageType> ResampleImageFilterType;
	typename ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();
	//typedef itk::BSplineInterpolateImageFunction<ImageType, double, double> T_Interpolator;
	typedef typename itk::LinearInterpolateImageFunction<ImageType, double> T_Interpolator;
  // Resize
	typename T_Interpolator::Pointer _pInterpolator = T_Interpolator::New();
	//_pInterpolator->SetSplineOrder(3);

	const typename ImageType::SizeType inputSize = image->GetLargestPossibleRegion().GetSize();
	typename ImageType::SizeType outputSize;
	const typename ImageType::SpacingType inputSpacing = image->GetSpacing();
	typename ImageType::SpacingType outputSpacing;
	
	const double scaling = std::floor((static_cast<double>(inputSpacing[2]))/(static_cast<double>(inputSpacing[0])));

	outputSpacing[0] = inputSpacing[0]; 
	outputSpacing[1] = inputSpacing[1]; 
	outputSpacing[2] = static_cast<double>(inputSpacing[2])/scaling; 
	typedef typename ImageType::SizeType::SizeValueType SizeValueType;
	outputSize[0] = (static_cast<double>(inputSize[0])); 
	outputSize[1] = (static_cast<double>(inputSize[1]));
	//outputSize[2] = ((static_cast<double>(inputSize[2]) -1 ) * scaling)+1;
	outputSize[2] = static_cast<SizeValueType>((static_cast<double>(inputSize[2]) * scaling) + .5);
	typename ImageType::PointType origin;
	origin = image->GetOrigin();
	origin[2] -= 0.5*inputSpacing[2];
	resample->SetOutputOrigin ( origin);


	resample->SetInput(image);
	resample->SetInterpolator(_pInterpolator);
	resample->SetSize(outputSize);
	resample->SetOutputSpacing(outputSpacing);
 	resample->SetTransform(TransformType::New());
	resample->UpdateLargestPossibleRegion();
	
	
	outputImage= resample->GetOutput();
	outputImage->Update();
	outputImage->DisconnectPipeline();
	//std::cout << "output image:\n"<< outputImage<< "\n";
	return outputImage;
}



template<typename PointSetType>
typename PointSetType::PointType::VectorType generateRandomVector(double length)
{
	typename PointSetType::PointType::VectorType vector;
	vector.Fill(0);
	
	typedef  typename itk::Statistics::MersenneTwisterRandomVariateGenerator RandomizerType;
    typename RandomizerType::Pointer randomizer = RandomizerType::New();
    randomizer->SetSeed(time(NULL));


	float angle1 = randomizer->GetUniformVariate(0,2*3.1415);
	//std::cout << "hello" <<angle1<<"\n";
	double x0 = std::cos(angle1);
	double y0 = std::sin(angle1);
	float z = randomizer->GetUniformVariate(-1, 1); // z is in the range [-1,1]  
	x0 = x0 * sqrt(1 - z*z);
	y0 = y0 * sqrt(1 - z*z);
	double z0 = z;

	float r = sqrt(x0*x0+y0*y0+z*z);

	vector.SetElement(0, length*x0);
	vector.SetElement(1, length*y0);
	vector.SetElement(2, length*z);
	return vector;
}