#ifndef __nrnGloabals_h
#define __nrnGloabals_h

#include <string>
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include <fstream>
#include "itkNumericTraits.h"

template<typename ImageType>
typename ImageType::Pointer nrnLoadImage(std::string filename);

template<typename ImageType>
void nrnSaveImage(std::string filename, typename ImageType::Pointer image);

template<typename ImageType, typename TransformType>
typename ImageType::Pointer resampleImage(typename ImageType::Pointer image,typename ImageType::Pointer referenceImage);

template<typename ImageType, typename TransformType>
typename ImageType::Pointer resampleImage(typename ImageType::Pointer image,
										  typename ImageType::Pointer referenceImage,
										  typename TransformType::Pointer transform	);

template<typename PointSetType>
typename PointSetType::Pointer nrnLoadLandmarks(std::string filename);

template<typename PointSetType>
void nrnSaveLandmarks(typename PointSetType::Pointer landmarks, std::string filename);

template<typename ImageType, typename TransformType>
void nrnSaveTransformationFile(typename ImageType::Pointer image, 
								typename TransformType::Pointer transform,
								std::string filename);

template<typename PointSetType>
void nrnPrintLandmarks(typename PointSetType::Pointer landmarks);

template<typename PointSetType>
typename PointSetType::Pointer displaceLandmarks(typename PointSetType::Pointer landmarks, 
												typename PointSetType::PointType::VectorType displacement);

template<typename PointSetType,typename ImageType>
typename PointSetType::Pointer createReferenceLandmarks( typename ImageType::Pointer referenceImage);

template<typename PointSetType,typename TransformType>
typename PointSetType::Pointer createMovingLandmarks( typename PointSetType::Pointer inputLandmarks, 
													  typename TransformType::Pointer transform,
													  typename PointSetType::PointType::VectorType displacement);// =
											//		 itk::NumericTraits<typename PointSetType::PointType::VectorType>::Zero);

template<typename ImageType>
typename ImageType::Pointer OtsuFilter(typename ImageType::Pointer image);

template<typename ImageType>
typename ImageType::Pointer GradientImageFilter(typename ImageType::Pointer image);

template<typename ImageType>
typename ImageType::Pointer smoothImage(typename ImageType::Pointer image, double sigma);

template<typename ImageType>
typename ImageType::Pointer smooth2Image(typename ImageType::Pointer image, double sigma);

template<typename ImageType>
typename ImageType::Pointer smooth3Image(typename ImageType::Pointer image, double sigma);

template<typename ImageType>
typename ImageType::Pointer smoothAndScalePlane(typename ImageType::Pointer image, double factor);

template<typename ImageType>
typename ImageType::Pointer shrinkImage(typename ImageType::Pointer image, double factor);


template<typename ImageType>
typename ImageType::Pointer shrinkImage2(typename ImageType::Pointer image, double factor);

template<typename ImageType>
typename ImageType::Pointer generateIsotropicSpacing(typename ImageType::Pointer image);

template<typename PointSetType>
typename PointSetType::PointType::VectorType generateRandomVector(double length);

#endif