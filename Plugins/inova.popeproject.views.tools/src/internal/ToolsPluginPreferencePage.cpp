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
#include <QSpacerItem>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>


ToolsPluginPreferencePage::ToolsPluginPreferencePage(QWidget*, Qt::WindowFlags)
//	: m_MainControl(nullptr), m_Initializing(false)
{}

void ToolsPluginPreferencePage::Init(berry::IWorkbench::Pointer)
{}

void ToolsPluginPreferencePage::CreateQtControl(QWidget* parent)
{
    m_Initializing = true;
    berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
    m_ToolsPluginPreferencesNode = prefService->GetSystemPreferences()->Node("/inova.popeproject.views.tools");

    m_MainControl = new QWidget(parent);

    auto formLayout = new QFormLayout;
    formLayout->setHorizontalSpacing(8);
    formLayout->setVerticalSpacing(24);

    auto displayOptionsLayout = new QVBoxLayout;
    m_RadioButton_3DAutoRotation = new QRadioButton("Enable 3D auto-rotation when loading data", m_MainControl);
    displayOptionsLayout->addWidget(m_RadioButton_3DAutoRotation);
    m_RadioButton_Std4ViewWidget = new QRadioButton("Enable standard 4-view widget when loading data", m_MainControl);
    displayOptionsLayout->addWidget(m_RadioButton_Std4ViewWidget);
    auto v_spacer = new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    displayOptionsLayout->addItem(v_spacer);
    m_CheckBox_ShowCrosshair = new QCheckBox("Show crosshair", m_MainControl);
    displayOptionsLayout->addWidget(m_CheckBox_ShowCrosshair);
    formLayout->addRow("View by default", displayOptionsLayout);

    m_CheckBox_VolumeRendering = new QCheckBox("Enable Volume Rendering when loading data", m_MainControl);
    formLayout->addRow("Volume by default", m_CheckBox_VolumeRendering);

    auto viewOptionsLayout = new QVBoxLayout;
    m_CheckBox_ShowPatientData = new QCheckBox("Show patient data", m_MainControl);
    viewOptionsLayout->addWidget(m_CheckBox_ShowPatientData);
    m_CheckBox_GroupTags = new QCheckBox("Group tags", m_MainControl);
    viewOptionsLayout->addWidget(m_CheckBox_GroupTags);
    m_CheckBox_ShowStatistics = new QCheckBox("Show statistics", m_MainControl);
    viewOptionsLayout->addWidget(m_CheckBox_ShowStatistics);
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
    m_ToolsPluginPreferencesNode->PutBool("show crosshair", m_CheckBox_ShowCrosshair->isChecked());
    m_ToolsPluginPreferencesNode->PutBool("volume rendering", m_CheckBox_VolumeRendering->isChecked());
    m_ToolsPluginPreferencesNode->PutBool("show patient data", m_CheckBox_ShowPatientData->isChecked());
    m_ToolsPluginPreferencesNode->PutBool("group tags", m_CheckBox_GroupTags->isChecked());
    m_ToolsPluginPreferencesNode->PutBool("show statistics", m_CheckBox_ShowStatistics->isChecked());
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
    m_CheckBox_ShowCrosshair->setChecked(m_ToolsPluginPreferencesNode->GetBool("show crosshair", false));
    m_CheckBox_VolumeRendering->setChecked(m_ToolsPluginPreferencesNode->GetBool("volume rendering", true));
    m_CheckBox_ShowPatientData->setChecked(m_ToolsPluginPreferencesNode->GetBool("show patient data", true));
    m_CheckBox_GroupTags->setChecked(m_ToolsPluginPreferencesNode->GetBool("group tags", true));
    m_CheckBox_ShowStatistics->setChecked(m_ToolsPluginPreferencesNode->GetBool("show statistics", false));
    m_CheckBox_EditableText->setChecked(m_ToolsPluginPreferencesNode->GetBool("editable text", false));
}
