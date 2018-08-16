#include "PopePreferencePage.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QCoreApplication>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>


PopePreferencePage::PopePreferencePage(QWidget* /*parent*/, Qt::WindowFlags)
//	: m_MainControl(nullptr), m_Initializing(false)
{}

void PopePreferencePage::Init(berry::IWorkbench::Pointer)
{}

void PopePreferencePage::CreateQtControl(QWidget* parent)
{
	m_Initializing = true;
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	m_PopePreferencesNode = prefService->GetSystemPreferences()->Node("/inova.popeproject.editors.renderwindow");

	m_MainControl = new QWidget(parent);

	auto formLayout = new QFormLayout;
	formLayout->setHorizontalSpacing(8);
	formLayout->setVerticalSpacing(24);

	m_CheckBox_SaveSession = new QCheckBox("Restore session after restarting (without data)", m_MainControl);
	formLayout->addRow("Session", m_CheckBox_SaveSession);

	auto controlElementsOptionsLayout = new QVBoxLayout;
	m_ComboBox_CloseablePlugins = new QComboBox(m_MainControl);
	m_ComboBox_CloseablePlugins->clear();
	m_ComboBox_CloseablePlugins->insertItems(0, QStringList() << "Not closeable" << "All but main are closeable" << "All closeable");
	m_ComboBox_CloseablePlugins->setCurrentIndex(CloseableMoveablePlugins::NotForAll);
	controlElementsOptionsLayout->addWidget(m_ComboBox_CloseablePlugins);
	m_ComboBox_MoveablePlugins = new QComboBox(m_MainControl);
	m_ComboBox_MoveablePlugins->clear();
	m_ComboBox_MoveablePlugins->insertItems(0, QStringList() << "Not moveable" << "All but main are moveable" << "All moveable");
	m_ComboBox_MoveablePlugins->setCurrentIndex(CloseableMoveablePlugins::YesForAllButMain);
	controlElementsOptionsLayout->addWidget(m_ComboBox_MoveablePlugins);
	formLayout->addRow("Plugins", controlElementsOptionsLayout);
	//formLayout->addRow("Plugins Closeable", m_ComboBox_CloseablePlugins);
	//formLayout->addRow("Plugins Moveable", m_ComboBox_MoveablePlugins);

	m_CheckBox_LoadRNND = new QCheckBox("Load RNND files", m_MainControl);
	formLayout->addRow("RNND", m_CheckBox_LoadRNND);

	m_LineEdit_DefaultPath = new QLineEdit("Default path", m_MainControl);
	formLayout->addRow("Data folder", m_LineEdit_DefaultPath);

	m_MainControl->setLayout(formLayout);
	this->Update();
	m_Initializing = false;
}

QWidget* PopePreferencePage::GetQtControl() const
{
	return m_MainControl;
}


/// Invoked when the OK button was clicked in the preferences dialog
bool PopePreferencePage::PerformOk()
{
	m_PopePreferencesNode->PutBool("save session", m_CheckBox_SaveSession->isChecked());
	m_PopePreferencesNode->PutInt("closeable plugins", m_ComboBox_CloseablePlugins->currentIndex());
	m_PopePreferencesNode->PutInt("moveable plugins", m_ComboBox_MoveablePlugins->currentIndex());
	m_PopePreferencesNode->PutBool("load rnnd", m_CheckBox_LoadRNND->isChecked());
	m_PopePreferencesNode->Put("data folder", m_LineEdit_DefaultPath->text());
	return true;
}
/// Invoked when the Cancel button was clicked in the preferences dialog
void PopePreferencePage::PerformCancel()
{}
void PopePreferencePage::Update()
{
	m_CheckBox_SaveSession->setChecked(m_PopePreferencesNode->GetBool("save session", true));

	m_ComboBox_CloseablePlugins->setCurrentIndex(m_PopePreferencesNode->GetInt("closeable plugins", CloseableMoveablePlugins::NotForAll));
	m_ComboBox_MoveablePlugins->setCurrentIndex(m_PopePreferencesNode->GetInt("moveable plugins", CloseableMoveablePlugins::YesForAllButMain));

	m_CheckBox_LoadRNND->setChecked(m_PopePreferencesNode->GetBool("load rnnd", false));

	QString def_path = QCoreApplication::applicationDirPath();
	m_LineEdit_DefaultPath->setText(m_PopePreferencesNode->Get("data folder", def_path));
}
