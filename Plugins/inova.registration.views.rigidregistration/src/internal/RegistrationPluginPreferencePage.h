//#pragma once
#ifndef REGISTRATIONPLUGINPREFERENCEPAGE_H
#define REGISTRATIONPLUGINPREFERENCEPAGE_H

#include <berryIPreferences.h>
#include <berryIQtPreferencePage.h>

using namespace std;

class QWidget;
class QCheckBox;
class QRadioButton;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QHBoxLayout;
class QLabel;
class QSpacerItem;


class RegistrationPluginPreferencePage : public QObject, public berry::IQtPreferencePage
{
	Q_OBJECT
	Q_INTERFACES(berry::IPreferencePage)

public:
	/// Build UI in constructor
	RegistrationPluginPreferencePage(QWidget* parent = 0, Qt::WindowFlags f = 0);
	/// Invoked when the OK button was clicked in the preferences dialog
	virtual bool PerformOk() override;
	/// Invoked when the Cancel button was clicked in the preferences dialog
	virtual void PerformCancel() override;
	/// \see IPreferencePage::Update()
	virtual void Update() override;

	void Init(berry::IWorkbench::Pointer workbench) override;
	void CreateQtControl(QWidget* widget) override;
	QWidget* GetQtControl() const override;

	void resize_ShrinkFactorsPerLevel(size_t numberOfLevels);
	void resize_SmoothingSigmasPerLevel(size_t numberOfLevels);
	void updateNumberOfLevels(size_t numberOfLevels);

protected slots:
	//void OnVolumeRenderingCheckboxChecked(int);
	//void OnSmoothingCheckboxChecked(int);
	void on_SpinBox_numberOfLevels_valueChanged(int);

protected:
	QWidget* m_MainControl = nullptr;
	berry::IPreferences::Pointer m_RegistrationPluginPreferencesNode;

	QSpinBox* m_SpinBox_numberOfLevels;
	QHBoxLayout* shrinkFactorsPerLevelLayout;
	vector<QSpinBox*> m_vector_SpinBox_shrinkFactorsPerLevel;
	vector<QLabel*> m_vector_QLabel_shrinkFactorsPerLevel;
	vector<QSpacerItem*> m_vector_QSpacerItem_shrinkFactorsPerLevel;
	QHBoxLayout* smoothingSigmasPerLevelLayout;
	vector<QSpinBox*> m_vector_SpinBox_smoothingSigmasPerLevel;
	vector<QLabel*> m_vector_QLabel_smoothingSigmasPerLevel;
	vector<QSpacerItem*> m_vector_QSpacerItem_smoothingSigmasPerLevel;

	QSpinBox* m_SpinBox_histBins;
	QSpinBox* m_SpinBox_samplingPercentage;
	QSpinBox* m_SpinBox_maximumIterationsWithoutProgress;
	QSpinBox* m_SpinBox_numberOfIterations;
	QSpinBox* m_SpinBox_convergenceWindowSize;
};

#endif // REGISTRATIONPLUGINPREFERENCEPAGE_H