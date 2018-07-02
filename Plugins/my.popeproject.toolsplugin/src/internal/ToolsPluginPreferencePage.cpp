
#include "ToolsPluginPreferencePage.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QComboBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>


ToolsPluginPreferencePage::ToolsPluginPreferencePage(QWidget* parent, Qt::WindowFlags f)
//	: m_MainControl(nullptr), m_Initializing(false)
{}

void ToolsPluginPreferencePage::Init(berry::IWorkbench::Pointer)
{}

void ToolsPluginPreferencePage::CreateQtControl(QWidget* parent)
{
	m_Initializing = true;
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	m_ToolsPluginPreferencesNode = prefService->GetSystemPreferences()->Node("/my.popeproject.views.toolsplugin");

	m_MainControl = new QWidget(parent);

	auto formLayout = new QFormLayout;
	formLayout->setHorizontalSpacing(8);
	formLayout->setVerticalSpacing(24);

	auto displayOptionsLayout = new QVBoxLayout;
	m_RadioButton_3DAutoRotation = new QRadioButton("Enable 3D auto-rotation when loading data", m_MainControl);
	displayOptionsLayout->addWidget(m_RadioButton_3DAutoRotation);
	m_RadioButton_Std4ViewWidget = new QRadioButton("Enable standard 4-view widget when loading data", m_MainControl);
	displayOptionsLayout->addWidget(m_RadioButton_Std4ViewWidget);
	formLayout->addRow("Main window", displayOptionsLayout);

	auto viewOptionsLayout = new QVBoxLayout;
	m_CheckBox_ShowPatientData = new QCheckBox("Show patient data", m_MainControl);
	viewOptionsLayout->addWidget(m_CheckBox_ShowPatientData);
	m_CheckBox_GroupTags = new QCheckBox("Group tags", m_MainControl);
	viewOptionsLayout->addWidget(m_CheckBox_GroupTags);
	m_CheckBox_ShowStatistics = new QCheckBox("Show statistics", m_MainControl);
	viewOptionsLayout->addWidget(m_CheckBox_ShowStatistics);
	m_CheckBox_ShowHistogram = new QCheckBox("Show histogram", m_MainControl);
	viewOptionsLayout->addWidget(m_CheckBox_ShowHistogram);
	formLayout->addRow("View", viewOptionsLayout);

	m_CheckBox_EditableText = new QCheckBox("Set text editable", m_MainControl);
	formLayout->addRow("Edit Mode", m_CheckBox_EditableText);

	m_MainControl->setLayout(formLayout);
	this->Update();
	m_Initializing = false;
}

QWidget* ToolsPluginPreferencePage::GetQtControl() const
{
	return m_MainControl;
}


/// Invoked when the OK button was clicked in the preferences dialog
bool ToolsPluginPreferencePage::PerformOk()
{
	m_ToolsPluginPreferencesNode->PutBool("3D autorotation", m_RadioButton_3DAutoRotation->isChecked());
	m_ToolsPluginPreferencesNode->PutBool("show patient data", m_CheckBox_ShowPatientData->isChecked());
	m_ToolsPluginPreferencesNode->PutBool("group tags", m_CheckBox_GroupTags->isChecked());
	m_ToolsPluginPreferencesNode->PutBool("show statistics", m_CheckBox_ShowStatistics->isChecked());
	m_ToolsPluginPreferencesNode->PutBool("show histogram", m_CheckBox_ShowHistogram->isChecked());
	m_ToolsPluginPreferencesNode->PutBool("editable text", m_CheckBox_EditableText->isChecked());
	return true;
}
/// Invoked when the Cancel button was clicked in the preferences dialog
void ToolsPluginPreferencePage::PerformCancel()
{}
void ToolsPluginPreferencePage::Update()
{
	if (m_ToolsPluginPreferencesNode->GetBool("3D autorotation", true))
	{
		m_RadioButton_3DAutoRotation->setChecked(true);
	}
	else
	{
		m_RadioButton_Std4ViewWidget->setChecked(true);
	}
	m_CheckBox_ShowPatientData->setChecked(m_ToolsPluginPreferencesNode->GetBool("show patient data", true));
	m_CheckBox_GroupTags->setChecked(m_ToolsPluginPreferencesNode->GetBool("group tags", true));
	m_CheckBox_ShowStatistics->setChecked(m_ToolsPluginPreferencesNode->GetBool("show statistics", false));
	m_CheckBox_ShowHistogram->setChecked(m_ToolsPluginPreferencesNode->GetBool("show histogram", false));
	m_CheckBox_EditableText->setChecked(m_ToolsPluginPreferencesNode->GetBool("editable text", false));
}
