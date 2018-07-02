
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


MainApplicationPreferencePage::ViewDescriptor::ViewDescriptor(QCheckBox* checkBox, const Elements::PluginDescriptor* plugin)
	: checkBox(checkBox), plugin(plugin)
{}


MainApplicationPreferencePage::MainApplicationPreferencePage(QWidget* parent, Qt::WindowFlags f)
{}

void MainApplicationPreferencePage::Init(berry::IWorkbench::Pointer)
{}

void MainApplicationPreferencePage::CreateQtControl(QWidget* parent)
{
	m_Initializing = true;
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	m_MainApplicationPreferencesNode = prefService->GetSystemPreferences()->Node("/my.popeproject.mainapplication");

	m_MainControl = new QWidget(parent);

	auto formLayout = new QFormLayout;
	formLayout->setHorizontalSpacing(8);
	formLayout->setVerticalSpacing(24);

	//m_CheckBox_ShowShortcut = new QCheckBox("Show the DICOM View in Menu and ToolBar", m_MainControl);
	//formLayout->addRow("Shortcut", m_CheckBox_ShowShortcut);
	auto viewOptionsLayout = new QVBoxLayout;
	viewOptionsLayout->setSpacing(6);
	for (auto plugin : *Elements::plugins_by_order())
	{
		QCheckBox* checkBox = new QCheckBox(plugin->name, m_MainControl);
		viewOptionsLayout->addWidget(checkBox);
		m_Views.push_back(ViewDescriptor(checkBox, plugin));
	}
	formLayout->addRow("Tabs", viewOptionsLayout);

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
}
