#include "ImageRegistrationCalculationThread.h"

//QT headers
#include <QMessageBox>
#include <QApplication>
#include <mitkImageMaskGenerator.h>
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>

// itk includes
#include "itkImageRegistrationMethodv4.h"
#include "itkMattesMutualInformationImageToImageMetricv4.h"
#include "itkVersorRigid3DTransform.h"
#include "itkQuasiNewtonOptimizerv4.h"
#include "itkTimeProbe.h"
#include "itkCommand.h"

// function collection includes
//#include "nrnGlobals.h"
#include "nrnGlobals.cpp"

// system includes
#include <sys/stat.h>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <iomanip>

// MITK
#include <mitkProgressBar.h>
#include <mitkImageCast.h>


// zur Anzeige der Iterationen während der Optimierung:
class CommandIterationUpdate : public itk::Command
{
public:
    typedef CommandIterationUpdate Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    itkNewMacro(Self);

protected:
    CommandIterationUpdate() {};

public:
    typedef itk::QuasiNewtonOptimizerv4 OptimizerType;
    typedef const OptimizerType* OptimizerPointer;
    void Execute(itk::Object* caller, const itk::EventObject& event) ITK_OVERRIDE
    {
        Execute((const itk::Object*)caller, event);
    }
    void Execute(const itk::Object* object, const itk::EventObject& event) ITK_OVERRIDE
    {
        OptimizerPointer optimizer = static_cast<OptimizerPointer>(object);
        if (!itk::IterationEvent().CheckEvent(&event))
        {
            return;
        }
        auto iteration = optimizer->GetCurrentIteration();
        std::cout << iteration << "   ";
        std::cout << optimizer->GetValue() << "   ";
        std::cout << optimizer->GetCurrentPosition() << std::endl;

        auto progressbar = mitk::ProgressBar::GetInstance();
        progressbar->Progress(1U);
    }
};

ImageRegistrationCalculationThread::ImageRegistrationCalculationThread()
    : QThread()
    , moving_image(nullptr)
    , target_image(nullptr)
    , transform(nullptr)
    , result_image(nullptr)
    , timeStep(0)
    , registrationChanged(false)
    , calculationSuccessful(false)
    , m_abort(false)
{
}

ImageRegistrationCalculationThread::~ImageRegistrationCalculationThread()
{
}

void ImageRegistrationCalculationThread::Initialize(mitk::Image::Pointer moving_image, mitk::Image::Pointer target_image)
{
    // reset old values
    if (this->moving_image.IsNotNull())
        this->moving_image = nullptr;
    if (this->target_image.IsNotNull())
        this->target_image = nullptr;

    // set new values if passed in
    if (moving_image.IsNotNull())
        this->moving_image = moving_image->Clone();
    if (target_image.IsNotNull())
        this->target_image = target_image->Clone();
}

void ImageRegistrationCalculationThread::SetTimeStep(int times)
{
    this->timeStep = times;
}

int ImageRegistrationCalculationThread::GetTimeStep()
{
    return this->timeStep;
}

void ImageRegistrationCalculationThread::SetSettings(const RegistrationSettings& settings)
{
    this->settings = settings;
}

TransformType::Pointer ImageRegistrationCalculationThread::GetTransform()
{
    return this->transform;
}

mitk::Image::Pointer ImageRegistrationCalculationThread::GetResultImage()
{
    return this->result_image;
}

std::string ImageRegistrationCalculationThread::GetLastErrorMessage()
{
    return m_message;
}

bool ImageRegistrationCalculationThread::GetRegistrationChangedFlag()
{
    return registrationChanged;
}

bool ImageRegistrationCalculationThread::GetRegistrationUpdateSuccessFlag()
{
    return calculationSuccessful;
}

//template <typename TPixel, unsigned int VImageDimension>
//void imageRegistration(itk::Image<TPixel, VImageDimension>* itkImage, SomeType param)
//{}
void ImageRegistrationCalculationThread::run()
{
    this->calculationSuccessful = true;

    if (moving_image == nullptr || target_image == nullptr)
    {
        MITK_INFO << "No images to start Registration";
        this->calculationSuccessful = false;
        return;
    }
    if (settings.numberOfLevels > settings.shrinkFactorsPerLevel.size())
    {
        MITK_INFO << "Settings are not consistent: Number of Levels > size of Shrink Factors Per Level";
        this->calculationSuccessful = false;
        return;
    }
    if (settings.numberOfLevels > settings.smoothingSigmasPerLevel.size())
    {
        MITK_INFO << "Settings are not consistent: Number of Levels > size of Smoothing Sigmas Per Level";
        this->calculationSuccessful = false;
        return;
    }

    try
    {
        const unsigned int Dimension = 3;
        typedef double PixelType;

        // load data:
        using ImageType = itk::Image<double, 3>;
        ImageType::Pointer fixedImage;// = nrnLoadImage<ImageType>("C:/FP/Allgemein/Daten/training_001/ct/training_001_ct.mhd");
        ImageType::Pointer movingImage;// = nrnLoadImage<ImageType>("C:/FP/Allgemein/Daten/training_001/mr_T2/training_001_mr_T2.mhd");
        mitk::CastToItkImage(moving_image, movingImage);
        mitk::CastToItkImage(target_image, fixedImage);

        // define transform:
        TransformType::Pointer transform = TransformType::New();
        using FixedImageType = itk::Image<PixelType, Dimension>;
        using MovingImageType = itk::Image<PixelType, Dimension>;
        // define optimizer and metric:
        using OptimizerType = itk::QuasiNewtonOptimizerv4;
        using MetricType = itk::MattesMutualInformationImageToImageMetricv4<FixedImageType, MovingImageType>;
        using RegistrationType = itk::ImageRegistrationMethodv4<FixedImageType, MovingImageType, TransformType>;

        MetricType::Pointer metric = MetricType::New();
        OptimizerType::Pointer optimizer = OptimizerType::New();
        RegistrationType::Pointer registration = RegistrationType::New();

        metric->SetNumberOfHistogramBins(settings.histBins);
        registration->SetMetric(metric);
        registration->SetOptimizer(optimizer);
        registration->SetFixedImage(fixedImage);
        registration->SetMovingImage(movingImage);
        registration->SetInitialTransform(transform);
//		double Pixel = fixedImage->GetLargestPossibleRegion().GetNumberOfPixels();
        using RegistrationParameterScalesFromShiftType = itk::RegistrationParameterScalesFromPhysicalShift<MetricType>;
        typename RegistrationParameterScalesFromShiftType::Pointer shiftScaleEstimator = RegistrationParameterScalesFromShiftType::New();
        shiftScaleEstimator->SetMetric(metric);
        optimizer->SetMetric(metric);
        optimizer->SetMaximumIterationsWithoutProgress(settings.maximumIterationsWithoutProgress);
        optimizer->SetNumberOfIterations(settings.numberOfIterations);
        optimizer->SetScalesEstimator(shiftScaleEstimator);
        optimizer->SetConvergenceWindowSize(settings.convergenceWindowSize);
        optimizer->ReturnBestParametersAndValueOn();

        CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
        optimizer->AddObserver(itk::IterationEvent(), observer);
        // multi Resolution:
        RegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
        RegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel;

        shrinkFactorsPerLevel.SetSize(settings.numberOfLevels);
        smoothingSigmasPerLevel.SetSize(settings.numberOfLevels);
        for (size_t i = 0; i < settings.numberOfLevels; i++)
        {
            shrinkFactorsPerLevel[i] = settings.shrinkFactorsPerLevel[i];
            smoothingSigmasPerLevel[i] = settings.smoothingSigmasPerLevel[i];
        }

        registration->SetNumberOfLevels(settings.numberOfLevels);
        registration->SetSmoothingSigmasPerLevel(smoothingSigmasPerLevel);
        registration->SetShrinkFactorsPerLevel(shrinkFactorsPerLevel);

        typename RegistrationType::MetricSamplingStrategyType samplingStrategy = RegistrationType::RANDOM;
        registration->SetMetricSamplingPercentage(settings.samplingPercentage);
        registration->SetMetricSamplingStrategy(samplingStrategy);

        auto progressbar = mitk::ProgressBar::GetInstance();
        progressbar->Reset();
        progressbar->AddStepsToDo(settings.numberOfIterations);

        double optTime;
        try
        {
            itk::TimeProbe clock;
            clock.Start();
            registration->Update();
            clock.Stop();
            optTime = clock.GetTotal();
            std::cout << "Time: " << optTime << "\n";
        }
        catch (itk::ExceptionObject & err)
        {
            std::cerr << "ExceptionObject caught !" << std::endl;
            std::cerr << err << std::endl;
            progressbar->Reset();
            this->calculationSuccessful = false;
            return;
        }

        const TransformType::ParametersType finalParameters = registration->GetOutput()->Get()->GetParameters();

        /*
        const double versorX = finalParameters[0];
        const double versorY = finalParameters[1];
        const double versorZ = finalParameters[2];
        const double finalTranslationX = finalParameters[3];
        const double finalTranslationY = finalParameters[4];
        const double finalTranslationZ = finalParameters[5];
        const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
        const double bestValue = optimizer->GetValue();
         */
        transform = TransformType::New();
        transform->SetFixedParameters(registration->GetOutput()->Get()->GetFixedParameters());
        transform->SetParameters(finalParameters);

        ImageType::Pointer resampledImage = ::resampleImage<ImageType, TransformType>(movingImage, fixedImage, transform);
        mitk::CastToMitkImage(resampledImage, result_image);
        progressbar->Reset();
    }
    catch (const mitk::Exception& e)
    {
        MITK_ERROR << "MITK Exception: " << e.what();
        this->calculationSuccessful = false;
    }
    catch (const itk::ExceptionObject& e)
    {
        MITK_ERROR << "ITK Exception:" << e.what();
        this->calculationSuccessful = false;
    }
    catch (const std::runtime_error &e)
    {
        MITK_ERROR << "Runtime Exception: " << e.what();
        this->calculationSuccessful = false;
    }
    catch (const std::exception &e)
    {
        MITK_ERROR << "Standard Exception: " << e.what();
        this->calculationSuccessful = false;
    }

    this->registrationChanged = false;

    if (result_image == nullptr)
    {
        MITK_WARN << "Registration calculation is stopped because the image was removed.";
        this->calculationSuccessful = false;
        return;
    }
    /*auto timeSteps = moving_image->GetTimeSteps();
    for (unsigned int i = 0; i < timeSteps; i++)
    {
        try
        {

        }
        catch (mitk::Exception& e)
        {
            MITK_ERROR << "MITK Exception: " << e.what();
            this->calculationSuccessful = false;
        }
        catch (const std::runtime_error &e)
        {
            MITK_ERROR << "Runtime Exception: " << e.what();
            this->calculationSuccessful = false;
        }
        catch (const std::exception &e)
        {
            MITK_ERROR << "Standard Exception: " << e.what();
            this->calculationSuccessful = false;
        }
    }

    if(this->calculationSuccessful)
    {

    }*/
}

void ImageRegistrationCalculationThread::Abort()
{
    m_abort = true;
}
