#ifndef ImageRegistrationCalculationThread_H_INCLUDED
#define ImageRegistrationCalculationThread_H_INCLUDED

//QT headers
#include <QThread>
#include <QEvent>

//mitk headers
#include <mitkImage.h>

#include "itkVersorRigid3DTransform.h"

#include <vector>

using namespace std;

typedef itk::VersorRigid3DTransform<double> TransformType;

struct RegistrationSettings
{
	unsigned int numberOfLevels = 3;
	unsigned int histBins = 250;
	double samplingPercentage = 0.9;
	itk::SizeValueType maximumIterationsWithoutProgress = 50;
	itk::SizeValueType numberOfIterations = 500;
	itk::SizeValueType convergenceWindowSize = 50;
	vector<unsigned int> shrinkFactorsPerLevel = {2, 2, 1};
	vector<unsigned int> smoothingSigmasPerLevel = {2, 1, 0};
};

/** /brief This class is executed as background thread for image registration calculation.
  * Documentation: This class is derived from QThread and is intended to run the image registration calculation 
  * in a background thread keepung the gui usable.
  */
class ImageRegistrationCalculationThread : public QThread
{
  Q_OBJECT

public:
	/*!
	/brief standard constructor. */
	ImageRegistrationCalculationThread();
	/*!
	/brief standard destructor. */
	~ImageRegistrationCalculationThread();
	/*!
	/brief Initializes the object with necessary data. */
	void Initialize(mitk::Image::Pointer moving_image, mitk::Image::Pointer target_image);
	/*!
	/brief returns the calculated image transform. */
	TransformType::Pointer GetTransform();
	/*!
	/brief */
	mitk::Image::Pointer GetResultImage();
	mitk::Image::Pointer GetMovingImage();
	/*!
	/brief Set the time step of the image you want to process. */
	void SetTimeStep(int times);
	/*!
	/brief Get the time step of the image you want to process. */
	int GetTimeStep();
	void SetSettings(const RegistrationSettings& settings);
	/*!
	/brief Method called once the thread is executed. */
	void run() override;

	void Abort();

	bool GetRegistrationUpdateSuccessFlag();
	bool GetRegistrationChangedFlag();

	std::string GetLastErrorMessage();

protected:
	//member declaration
	RegistrationSettings settings;
	mitk::Image::Pointer moving_image;     ///< member variable holds the input image for which the registration needs to be calculated.
	mitk::Image::Pointer target_image;     ///< member variable holds the input image for which the registration needs to be calculated.
	TransformType::Pointer transform;      ///< member variable holds the result transform.
	mitk::Image::Pointer result_image;     ///< member variable holds the result image.
	int timeStep;                          ///< member variable holds the time step for statistics calculation
	bool registrationChanged;              ///< flag set if registration has changed
	bool calculationSuccessful;            ///< flag set if registration calculation was successful
	bool m_abort;
	std::string m_message;
};
#endif // ImageRegistrationCalculationThread_H_INCLUDED
