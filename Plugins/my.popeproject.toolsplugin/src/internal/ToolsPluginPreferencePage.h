//#pragma once
#ifndef TOOLSPLUGINPREFERENCEPAGE_H
#define TOOLSPLUGINPREFERENCEPAGE_H

#include <berryIPreferences.h>
#include <berryIQtPreferencePage.h>

using namespace std;

class QWidget;
class QCheckBox;
class QRadioButton;
class QDoubleSpinBox;
class QComboBox;


class ToolsPluginPreferencePage : public QObject, public berry::IQtPreferencePage
{
	Q_OBJECT
	Q_INTERFACES(berry::IPreferencePage)

public:
	/// Build UI in constructor
	ToolsPluginPreferencePage(QWidget* parent = 0, Qt::WindowFlags f = 0);
	/// Invoked when the OK button was clicked in the preferences dialog
	virtual bool PerformOk() override;
	/// Invoked when the Cancel button was clicked in the preferences dialog
	virtual void PerformCancel() override;
	/// \see IPreferencePage::Update()
	virtual void Update() override;


	void Init(berry::IWorkbench::Pointer workbench) override;
	void CreateQtControl(QWidget* widget) override;
	QWidget* GetQtControl() const override;

protected slots:
	//void OnVolumeRenderingCheckboxChecked(int);
	//void OnSmoothingCheckboxChecked(int);

protected:
	QWidget* m_MainControl = nullptr;
	bool m_Initializing = false;
	berry::IPreferences::Pointer m_ToolsPluginPreferencesNode;

	QRadioButton* m_RadioButton_3DAutoRotation;
	QRadioButton* m_RadioButton_Std4ViewWidget;

	QCheckBox* m_CheckBox_VolumeRendering;

	QCheckBox* m_CheckBox_ShowPatientData;
	QCheckBox* m_CheckBox_GroupTags;
	QCheckBox* m_CheckBox_ShowStatistics;
	QCheckBox* m_CheckBox_ShowHistogram;

	QCheckBox* m_CheckBox_EditableText;
};

#endif // TOOLSPLUGINPREFERENCEPAGE_H