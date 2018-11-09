
#include "RegistrationPluginPreferencePage.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSpacerItem>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <sstream>
#include <cmath>


RegistrationPluginPreferencePage::RegistrationPluginPreferencePage(QWidget* /*parent*/, Qt::WindowFlags)
//	: m_MainControl(nullptr)
{}

void RegistrationPluginPreferencePage::Init(berry::IWorkbench::Pointer)
{}

void RegistrationPluginPreferencePage::CreateQtControl(QWidget* parent)
{
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	m_RegistrationPluginPreferencesNode = prefService->GetSystemPreferences()->Node("/inova.registration.views.rigidregistration");

	m_MainControl = new QWidget(parent);

	auto formLayout = new QFormLayout;
	formLayout->setHorizontalSpacing(8);
	formLayout->setVerticalSpacing(24);

	m_SpinBox_numberOfLevels = new QSpinBox(m_MainControl);
	m_SpinBox_numberOfLevels->setMinimum(1);
	formLayout->addRow("Number Of Levels:", m_SpinBox_numberOfLevels);
	shrinkFactorsPerLevelLayout = new QHBoxLayout;
	//QLabel* label_shrinkFactorsPerLevel = new QLabel("Shrink Factors Per Level: ", m_MainControl);
	//shrinkFactorsPerLevelLayout->addWidget(label_shrinkFactorsPerLevel);
	formLayout->addRow("Shrink Factors Per Level: ", shrinkFactorsPerLevelLayout);
	smoothingSigmasPerLevelLayout = new QHBoxLayout;
	formLayout->addRow("Smoothing Sigmas Per Level: ", smoothingSigmasPerLevelLayout);
	m_SpinBox_histBins = new QSpinBox(m_MainControl);
	m_SpinBox_histBins->setMinimum(5);
	m_SpinBox_histBins->setMaximum(9999);
	formLayout->addRow("Histogram Bins:", m_SpinBox_histBins);
	m_SpinBox_samplingPercentage = new QSpinBox(m_MainControl);
	m_SpinBox_samplingPercentage->setMinimum(0);
	m_SpinBox_samplingPercentage->setMaximum(100);
	formLayout->addRow("Sampling Percentage:", m_SpinBox_samplingPercentage);
	m_SpinBox_maximumIterationsWithoutProgress = new QSpinBox(m_MainControl);
	m_SpinBox_maximumIterationsWithoutProgress->setMinimum(1);
	m_SpinBox_maximumIterationsWithoutProgress->setMaximum(1000);
	formLayout->addRow("Maximum Iterations Without Progress:", m_SpinBox_maximumIterationsWithoutProgress);
	m_SpinBox_numberOfIterations = new QSpinBox(m_MainControl);
	m_SpinBox_numberOfIterations->setMinimum(1);
	m_SpinBox_numberOfIterations->setMaximum(9999);
	formLayout->addRow("Number Of Iterations:", m_SpinBox_numberOfIterations);
	m_SpinBox_convergenceWindowSize = new QSpinBox(m_MainControl);
	m_SpinBox_convergenceWindowSize->setMinimum(1);
	m_SpinBox_convergenceWindowSize->setMaximum(9999);
	formLayout->addRow("Convergence Window Size:", m_SpinBox_convergenceWindowSize);

	connect(this->m_SpinBox_numberOfLevels, SIGNAL(valueChanged(int)), this, SLOT(on_SpinBox_numberOfLevels_valueChanged(int)));

	m_MainControl->setLayout(formLayout);
	this->Update();
}

QWidget* RegistrationPluginPreferencePage::GetQtControl() const
{
	return m_MainControl;
}


/// Invoked when the OK button was clicked in the preferences dialog
bool RegistrationPluginPreferencePage::PerformOk()
{
	int numberOfLevels = m_SpinBox_numberOfLevels->value();
	m_RegistrationPluginPreferencesNode->PutInt("numberOfLevels", numberOfLevels);
	for (int i = 0; i < numberOfLevels; i++)
	{
		stringstream s_shrinkFactorsPerLevel;
		s_shrinkFactorsPerLevel << "shrinkFactorsPerLevel_" << i;
		m_RegistrationPluginPreferencesNode->PutInt(QString::fromStdString(s_shrinkFactorsPerLevel.str()), m_vector_SpinBox_shrinkFactorsPerLevel[i]->value());
		stringstream s_smoothingSigmasPerLevel;
		s_smoothingSigmasPerLevel << "smoothingSigmasPerLevel_" << i;
		m_RegistrationPluginPreferencesNode->PutInt(QString::fromStdString(s_smoothingSigmasPerLevel.str()), m_vector_SpinBox_smoothingSigmasPerLevel[i]->value());
	}
	m_RegistrationPluginPreferencesNode->PutInt("histBins", m_SpinBox_histBins->value());
	float percentage = 0.01f * m_SpinBox_samplingPercentage->value();
	m_RegistrationPluginPreferencesNode->PutFloat("samplingPercentage", percentage);
	m_RegistrationPluginPreferencesNode->PutInt("maximumIterationsWithoutProgress", m_SpinBox_maximumIterationsWithoutProgress->value());
	m_RegistrationPluginPreferencesNode->PutInt("numberOfIterations", m_SpinBox_numberOfIterations->value());
	m_RegistrationPluginPreferencesNode->PutInt("convergenceWindowSize", m_SpinBox_convergenceWindowSize->value());

	return true;
}
/// Invoked when the Cancel button was clicked in the preferences dialog
void RegistrationPluginPreferencePage::PerformCancel()
{}
void RegistrationPluginPreferencePage::Update()
{
	int numberOfLevels = m_RegistrationPluginPreferencesNode->GetInt("numberOfLevels", 3);
	m_SpinBox_numberOfLevels->setValue(numberOfLevels); // --> will be called: updateNumberOfLevels(numberOfLevels); 

	m_SpinBox_histBins->setValue(m_RegistrationPluginPreferencesNode->GetInt("histBins", 250));
	int percentage = (int)std::round(100 * m_RegistrationPluginPreferencesNode->GetFloat("samplingPercentage", 0.9f));
	m_SpinBox_samplingPercentage->setValue(percentage);
	m_SpinBox_maximumIterationsWithoutProgress->setValue(m_RegistrationPluginPreferencesNode->GetInt("maximumIterationsWithoutProgress", 50));
	m_SpinBox_numberOfIterations->setValue(m_RegistrationPluginPreferencesNode->GetInt("numberOfIterations", 500));
	m_SpinBox_convergenceWindowSize->setValue(m_RegistrationPluginPreferencesNode->GetInt("convergenceWindowSize", 50));
}

void RegistrationPluginPreferencePage::resize_ShrinkFactorsPerLevel(size_t numberOfLevels)
{
	m_vector_SpinBox_shrinkFactorsPerLevel.resize(numberOfLevels);
	m_vector_QLabel_shrinkFactorsPerLevel.resize(numberOfLevels);
	m_vector_QSpacerItem_shrinkFactorsPerLevel.resize(numberOfLevels);
}
void RegistrationPluginPreferencePage::resize_SmoothingSigmasPerLevel(size_t numberOfLevels)
{
	m_vector_SpinBox_smoothingSigmasPerLevel.resize(numberOfLevels);
	m_vector_QLabel_smoothingSigmasPerLevel.resize(numberOfLevels);
	m_vector_QSpacerItem_smoothingSigmasPerLevel.resize(numberOfLevels);
}
void RegistrationPluginPreferencePage::updateNumberOfLevels(size_t numberOfLevels)
{
	size_t old_size_shrinkFactorsPerLevel = m_vector_SpinBox_shrinkFactorsPerLevel.size();
	if (old_size_shrinkFactorsPerLevel < numberOfLevels)
	{
		resize_ShrinkFactorsPerLevel(numberOfLevels);
		for (size_t i = old_size_shrinkFactorsPerLevel; i < numberOfLevels; i++)
		{
			m_vector_SpinBox_shrinkFactorsPerLevel[i] = new QSpinBox(m_MainControl);
			stringstream s_shrinkFactorsPerLevel;
			s_shrinkFactorsPerLevel << "shrinkFactorsPerLevel_" << i;
			QString name = QString::fromStdString(s_shrinkFactorsPerLevel.str());
			vector<unsigned int> def_shrinkFactorsPerLevel = { 2, 2, 1 };
			int def_val = i < def_shrinkFactorsPerLevel.size() ? def_shrinkFactorsPerLevel[i] : 1;
			int value = m_RegistrationPluginPreferencesNode->GetInt(name, def_val);
			m_vector_SpinBox_shrinkFactorsPerLevel[i]->setValue(value);
			stringstream ss;
			ss << ' ' << i + 1 << ':';
			m_vector_QLabel_shrinkFactorsPerLevel[i] = new QLabel(QString::fromStdString(ss.str()), m_MainControl);
			m_vector_QSpacerItem_shrinkFactorsPerLevel[i] = new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
			shrinkFactorsPerLevelLayout->addWidget(m_vector_QLabel_shrinkFactorsPerLevel[i]);
			shrinkFactorsPerLevelLayout->addWidget(m_vector_SpinBox_shrinkFactorsPerLevel[i]);
			shrinkFactorsPerLevelLayout->addItem(m_vector_QSpacerItem_shrinkFactorsPerLevel[i]);
		}
	}
	else if (old_size_shrinkFactorsPerLevel > numberOfLevels)
	{
		for (size_t i = numberOfLevels; i < old_size_shrinkFactorsPerLevel; i++)
		{
			shrinkFactorsPerLevelLayout->removeWidget(m_vector_QLabel_shrinkFactorsPerLevel[i]);
			shrinkFactorsPerLevelLayout->removeWidget(m_vector_SpinBox_shrinkFactorsPerLevel[i]);
			shrinkFactorsPerLevelLayout->removeItem(m_vector_QSpacerItem_shrinkFactorsPerLevel[i]);
			delete m_vector_QLabel_shrinkFactorsPerLevel[i];
			delete m_vector_SpinBox_shrinkFactorsPerLevel[i];
			delete m_vector_QSpacerItem_shrinkFactorsPerLevel[i];
		}
		resize_ShrinkFactorsPerLevel(numberOfLevels);
	}

	size_t old_size_smoothingSigmasPerLevel = m_vector_SpinBox_smoothingSigmasPerLevel.size();
	if (old_size_smoothingSigmasPerLevel < numberOfLevels)
	{
		resize_SmoothingSigmasPerLevel(numberOfLevels);
		for (size_t i = old_size_smoothingSigmasPerLevel; i < numberOfLevels; i++)
		{
			m_vector_SpinBox_smoothingSigmasPerLevel[i] = new QSpinBox(m_MainControl);
			stringstream s_smoothingSigmasPerLevel;
			s_smoothingSigmasPerLevel << "smoothingSigmasPerLevel_" << i;
			QString name = QString::fromStdString(s_smoothingSigmasPerLevel.str());
			vector<unsigned int> def_smoothingSigmasPerLevel = { 2, 1, 0 };
			int def_val = i < def_smoothingSigmasPerLevel.size() ? def_smoothingSigmasPerLevel[i] : 0;
			int value = m_RegistrationPluginPreferencesNode->GetInt(name, def_val);
			m_vector_SpinBox_smoothingSigmasPerLevel[i]->setValue(value);
			stringstream ss;
			ss << ' ' << i + 1 << ": ";
			m_vector_QLabel_smoothingSigmasPerLevel[i] = new QLabel(QString::fromStdString(ss.str()), m_MainControl);
			m_vector_QSpacerItem_smoothingSigmasPerLevel[i] = new QSpacerItem(10, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
			smoothingSigmasPerLevelLayout->addWidget(m_vector_QLabel_smoothingSigmasPerLevel[i]);
			smoothingSigmasPerLevelLayout->addWidget(m_vector_SpinBox_smoothingSigmasPerLevel[i]);
			smoothingSigmasPerLevelLayout->addItem(m_vector_QSpacerItem_smoothingSigmasPerLevel[i]);
		}
	}
	else if (old_size_smoothingSigmasPerLevel > numberOfLevels)
	{
		for (size_t i = numberOfLevels; i < old_size_shrinkFactorsPerLevel; i++)
		{
			smoothingSigmasPerLevelLayout->removeWidget(m_vector_QLabel_smoothingSigmasPerLevel[i]);
			smoothingSigmasPerLevelLayout->removeWidget(m_vector_SpinBox_smoothingSigmasPerLevel[i]);
			smoothingSigmasPerLevelLayout->removeItem(m_vector_QSpacerItem_smoothingSigmasPerLevel[i]);
			delete m_vector_QLabel_smoothingSigmasPerLevel[i];
			delete m_vector_SpinBox_smoothingSigmasPerLevel[i];
			delete m_vector_QSpacerItem_smoothingSigmasPerLevel[i];
		}
		resize_SmoothingSigmasPerLevel(numberOfLevels);
	}
}

void RegistrationPluginPreferencePage::on_SpinBox_numberOfLevels_valueChanged(int numberOfLevels)
{
	updateNumberOfLevels(numberOfLevels);
}
