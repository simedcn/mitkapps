
#include "MainApplicationPreferencePage.h"
#include "MainPerspective.h"

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


MainApplicationPreferencePage::ViewDescriptor::ViewDescriptor(QCheckBox* checkBox, const PluginDescriptor* plugin)
	: checkBox(checkBox), plugin(plugin)
{}


MainApplicationPreferencePage::MainApplicationPreferencePage(QWidget* /*parent*/, Qt::WindowFlags)
{}

void MainApplicationPreferencePage::Init(berry::IWorkbench::Pointer)
{}

void MainApplicationPreferencePage::CreateQtControl(QWidget* parent)
{
	m_Initializing = true;
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	m_MainApplicationPreferencesNode = prefService->GetSystemPreferences()->Node("/inova.popeproject.apps.mainapplication");

	m_MainControl = new QWidget(parent);

	auto formLayout = new QFormLayout;
	formLayout->setHorizontalSpacing(8);
	formLayout->setVerticalSpacing(16);

	//m_CheckBox_ShowShortcut = new QCheckBox("Show the DICOM View in Menu and ToolBar", m_MainControl);
	//formLayout->addRow("Shortcut", m_CheckBox_ShowShortcut);
	auto viewOptionsLayout = new QVBoxLayout;
	viewOptionsLayout->setSpacing(6);
	for (auto plugin : *PluginDescriptors::plugins_by_order())
	{
		QCheckBox* checkBox = new QCheckBox(plugin->name, m_MainControl);
		viewOptionsLayout->addWidget(checkBox);
		m_Views.push_back(ViewDescriptor(checkBox, plugin));
	}
	formLayout->addRow("Tabs", viewOptionsLayout);

	checkBox_showShortucts = new QCheckBox("Show shortcuts", m_MainControl);
	formLayout->addRow("ToolBar", checkBox_showShortucts);

	m_MainControl->setLayout(formLayout);
	this->Update();
	m_Initializing = false;
}

QWidget* MainApplicationPreferencePage::GetQtControl() const
{
	return m_MainControl;
}


/// Invoked when the OK button was clicked in the preferences dialog
bool MainApplicationPreferencePage::PerformOk()
{
	for (const auto& view : m_Views)
	{
		m_MainApplicationPreferencesNode->PutBool(view.plugin->id, view.checkBox->isChecked());
	}
	m_MainApplicationPreferencesNode->PutBool("show shortcuts", checkBox_showShortucts->isChecked());
	return true;
}
/// Invoked when the Cancel button was clicked in the preferences dialog
void MainApplicationPreferencePage::PerformCancel()
{}
void MainApplicationPreferencePage::Update()
{
	for (const auto& view : m_Views)
	{
		view.checkBox->setChecked(m_MainApplicationPreferencesNode->GetBool(view.plugin->id, view.plugin->is_open));
	}
	checkBox_showShortucts->setChecked(m_MainApplicationPreferencesNode->GetBool("show shortcuts", false));
}
