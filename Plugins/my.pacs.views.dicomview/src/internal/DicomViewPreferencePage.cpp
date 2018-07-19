
#include "DicomViewPreferencePage.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>


DicomViewPreferencePage::DicomViewPreferencePage(QWidget*, Qt::WindowFlags)
//	: m_MainControl(nullptr), m_Initializing(false)
{}

void DicomViewPreferencePage::Init(berry::IWorkbench::Pointer)
{}

void DicomViewPreferencePage::CreateQtControl(QWidget* parent)
{
    m_Initializing = true;
    berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

    m_DicomViewPreferencesNode = prefService->GetSystemPreferences()->Node("/my.pacs.views.dicomview");

    m_MainControl = new QWidget(parent);

    auto  formLayout = new QFormLayout;
    formLayout->setHorizontalSpacing(8);
    formLayout->setVerticalSpacing(24);

    /*auto serverSettingsLayout = new QVBoxLayout;
    m_lineEdit_IP = new QLineEdit("Host / IP address", m_MainControl);
    serverSettingsLayout->addWidget(m_lineEdit_IP);
    m_spinBox_Port = new QSpinBox(m_MainControl);
    serverSettingsLayout->addWidget(m_spinBox_Port);
    m_lineEdit_AETitle = new QLineEdit("AETitle", m_MainControl);
    serverSettingsLayout->addWidget(m_lineEdit_AETitle);
    m_comboBox_Protocol = new QComboBox(m_MainControl);
    serverSettingsLayout->addWidget(m_comboBox_Protocol);
    formLayout->addRow("Server Settings", serverSettingsLayout);*/
    m_lineEdit_IP = new QLineEdit("Host / IP address", m_MainControl);
    formLayout->addRow("Host / IP address", m_lineEdit_IP);
    m_spinBox_Port = new QSpinBox(m_MainControl);
    m_spinBox_Port->setMaximum(999999);
    m_spinBox_Port->setValue(11112);
    formLayout->addRow("Port", m_spinBox_Port);
    m_lineEdit_AETitle = new QLineEdit("AETitle", m_MainControl);
    formLayout->addRow("AETitle", m_lineEdit_AETitle);
    m_comboBox_Protocol = new QComboBox(m_MainControl);
    m_comboBox_Protocol->insertItems(0, QStringList() << "C-MOVE" << "C-GET");
    m_comboBox_Protocol->setCurrentIndex(0);
    formLayout->addRow("Retrieve protocol", m_comboBox_Protocol);
    m_lineEdit_StorageAETitle = new QLineEdit("Storage AETitle", m_MainControl);
    formLayout->addRow("Storage AETitle", m_lineEdit_StorageAETitle);

    m_MainControl->setLayout(formLayout);
    this->Update();
    m_Initializing = false;
}

QWidget* DicomViewPreferencePage::GetQtControl() const
{
    return m_MainControl;
}


/// Invoked when the OK button was clicked in the preferences dialog
bool DicomViewPreferencePage::PerformOk()
{
    m_DicomViewPreferencesNode->Put("PACS IP", m_lineEdit_IP->text());
    m_DicomViewPreferencesNode->PutInt("PACS port", m_spinBox_Port->value());
    m_DicomViewPreferencesNode->Put("PACS AETitle", m_lineEdit_AETitle->text());
    m_DicomViewPreferencesNode->PutInt("PACS protocol", m_comboBox_Protocol->currentIndex());
    m_DicomViewPreferencesNode->Put("PACS storage AETitle", m_lineEdit_StorageAETitle->text());
    return true;
}
/// Invoked when the Cancel button was clicked in the preferences dialog
void DicomViewPreferencePage::PerformCancel()
{}
void DicomViewPreferencePage::Update()
{
    m_lineEdit_IP->setText(m_DicomViewPreferencesNode->Get("PACS IP", "127.0.0.1"));
    m_spinBox_Port->setValue(m_DicomViewPreferencesNode->GetInt("PACS port", 11112));
    m_lineEdit_AETitle->setText(m_DicomViewPreferencesNode->Get("PACS AETitle", "SERVERAE"));
    m_comboBox_Protocol->setCurrentIndex(m_DicomViewPreferencesNode->GetInt("PACS protocol", 0));
    m_lineEdit_StorageAETitle->setText(m_DicomViewPreferencesNode->Get("PACS storage AETitle", "SERVERAE"));
}
