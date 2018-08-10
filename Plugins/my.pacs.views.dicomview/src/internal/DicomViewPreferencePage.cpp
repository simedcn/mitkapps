
#include "DicomViewPreferencePage.h"
#include "DicomView.h"

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
#include <QFileDialog>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>


DicomViewPreferencePage::DicomViewPreferencePage(QWidget* /*parent*/, Qt::WindowFlags)
//	: m_MainControl(nullptr), m_Initializing(false)
{}

void DicomViewPreferencePage::Init(berry::IWorkbench::Pointer)
{}

void DicomViewPreferencePage::CreateQtControl(QWidget* parent)
{
	m_Control = new QWidget(parent);
	ui.setupUi(m_Control);

	connect(this->ui.pushButton_PublicPACS_MedicalConnections, SIGNAL(clicked()), this, SLOT(on_pushButton_PublicPACS_MedicalConnections_clicked()));
	connect(this->ui.pushButton_PublicPACS_PixelMed, SIGNAL(clicked()), this, SLOT(on_pushButton_PublicPACS_PixelMed_clicked()));
	connect(this->ui.buttonGroup_Protocol, SIGNAL(buttonToggled(QAbstractButton*, bool)), this, SLOT(on_buttonGroup_Protocol_buttonToggled(QAbstractButton*, bool)));
	connect(this->ui.lineEdit_LocalFolderPath, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_LocalFolderPath_textChanged(QString)));
	connect(this->ui.pushButton_LocalFolderPath, SIGNAL(clicked()), this, SLOT(on_pushButton_LocalFolderPath_clicked()));
	connect(this->ui.pushButton_SetTemporaryLocalFolderPath, SIGNAL(clicked()), this, SLOT(on_pushButton_SetTemporaryLocalFolderPath_clicked()));

	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	m_DicomViewPreferencesNode = prefService->GetSystemPreferences()->Node("/my.pacs.views.dicomview");

	this->Update();
}

QWidget* DicomViewPreferencePage::GetQtControl() const
{
	return m_Control;
}


/// Invoked when the OK button was clicked in the preferences dialog
bool DicomViewPreferencePage::PerformOk()
{
	m_DicomViewPreferencesNode->Put("PACS host IP", ui.lineEdit_IP->text());
	m_DicomViewPreferencesNode->PutInt("PACS host port", ui.spinBox_Port->value());
	m_DicomViewPreferencesNode->Put("PACS host AETitle", ui.lineEdit_HostAETitle->text());
	int protocol = (ui.radioButton_Protocol_CGET->isChecked() ? PROTOCOL_CGET : PROTOCOL_CMOVE);
	m_DicomViewPreferencesNode->PutInt("PACS protocol", protocol);
	QString directory = ui.lineEdit_LocalFolderPath->text();
	if (directory == NO_DIRECTORY_SPECIFIED)
		directory = "";
	m_DicomViewPreferencesNode->PutBool("add to DataManager", ui.checkBox_AddToDataManager->isChecked());
	m_DicomViewPreferencesNode->Put("PACS storage local folder", directory);
	m_DicomViewPreferencesNode->Put("PACS storage local AETitle", ui.lineEdit_StorageAETitle->text());
	m_DicomViewPreferencesNode->Put("PACS storage local IP", ui.lineEdit_LocalStorageIP->text());
	m_DicomViewPreferencesNode->PutInt("PACS storage local port", ui.spinBox_LocalStoragePort->value());
	int destination = 
		is_savedDestination_LocalFolder ? DESTINATION_LOCALFOLDER
		: ui.radioButton_DestinationLocalFolder->isChecked() ? DESTINATION_LOCALFOLDER : DESTINATION_PACS;
	m_DicomViewPreferencesNode->PutInt("PACS destination", destination);
	m_DicomViewPreferencesNode->Put("PACS destination AETitle", ui.lineEdit_DestinationAETitle->text());

	return true;
}
/// Invoked when the Cancel button was clicked in the preferences dialog
void DicomViewPreferencePage::PerformCancel()
{}
void DicomViewPreferencePage::Update()
{
	vector<QObject*> ui_elements =
	{
		ui.lineEdit_IP,
		ui.spinBox_Port,
		ui.lineEdit_HostAETitle,
		ui.buttonGroup_Protocol,
		ui.buttonGroup_Destination,
		ui.lineEdit_DestinationAETitle,
		ui.checkBox_AddToDataManager,
		ui.lineEdit_LocalFolderPath,
		ui.pushButton_LocalFolderPath,
		ui.pushButton_SetTemporaryLocalFolderPath,
		ui.lineEdit_StorageAETitle,
		ui.lineEdit_LocalStorageIP,
		ui.spinBox_LocalStoragePort
	};
	for (auto ui_element : ui_elements)
	{
		ui_element->blockSignals(true);
	}
	QString text = m_DicomViewPreferencesNode->Get("PACS host IP", "dicomserver.co.uk");
	if (text != ui.lineEdit_IP->text())
		ui.lineEdit_IP->setText(text);
	int value = m_DicomViewPreferencesNode->GetInt("PACS host port", 104);
	if (value != ui.spinBox_Port->value())
		ui.spinBox_Port->setValue(value);
	text = m_DicomViewPreferencesNode->Get("PACS host AETitle", "server");
	if (text != ui.lineEdit_HostAETitle->text())
		ui.lineEdit_HostAETitle->setText(text);
	int protocol = m_DicomViewPreferencesNode->GetInt("PACS protocol", PROTOCOL_CGET);
	int destination = m_DicomViewPreferencesNode->GetInt("PACS destination", DESTINATION_LOCALFOLDER);
	if (destination == 0) // ! before checking C-GET
		ui.radioButton_DestinationLocalFolder->setChecked(true);
	else
		ui.radioButton_DestinationPACS->setChecked(true);
	if (protocol == PROTOCOL_CGET)
		ui.radioButton_Protocol_CGET->setChecked(true);
	else
		ui.radioButton_Protocol_CMOVE->setChecked(true);
	ui.checkBox_AddToDataManager->setChecked(m_DicomViewPreferencesNode->GetBool("add to DataManager", true));
	QString directory = m_DicomViewPreferencesNode->Get("PACS storage local folder", "");
	if (directory.isEmpty())
		directory = NO_DIRECTORY_SPECIFIED;
	if (directory != ui.lineEdit_LocalFolderPath->text())
		ui.lineEdit_LocalFolderPath->setText(directory);
	text = m_DicomViewPreferencesNode->Get("PACS storage local AETitle", "POPEAE");
	if (text != ui.lineEdit_StorageAETitle->text())
		ui.lineEdit_StorageAETitle->setText(text);
	text = m_DicomViewPreferencesNode->Get("PACS storage local IP", "127.0.0.1");
	if (text != ui.lineEdit_LocalStorageIP->text())
		ui.lineEdit_LocalStorageIP->setText(text);
	value = m_DicomViewPreferencesNode->GetInt("PACS storage local port", 11112);
	if (value != ui.spinBox_LocalStoragePort->value())
		ui.spinBox_LocalStoragePort->setValue(value);
	text = m_DicomViewPreferencesNode->Get("PACS destination AETitle", "ARCHIVESTATIONAE");
	if (text != ui.lineEdit_DestinationAETitle->text())
		ui.lineEdit_DestinationAETitle->setText(text);
	for (auto ui_element : ui_elements)
	{
		ui_element->blockSignals(false);
	}
	updateProtocolInUI();
}
void DicomViewPreferencePage::updateProtocolInUI()
{
	bool is_CGET = ui.radioButton_Protocol_CGET->isChecked();

	ui.radioButton_DestinationLocalFolder->setEnabled(is_CGET);

	if (!is_CGET && !is_savedDestination_LocalFolder)
	{
		int destination = (ui.radioButton_DestinationLocalFolder->isChecked() ? DESTINATION_LOCALFOLDER : DESTINATION_PACS);
		is_savedDestination_LocalFolder = (destination == DESTINATION_LOCALFOLDER);
		if (is_savedDestination_LocalFolder)
			ui.radioButton_DestinationPACS->setChecked(true);
	}
	else if (is_CGET && is_savedDestination_LocalFolder)
	{
		ui.radioButton_DestinationLocalFolder->setChecked(true);
		is_savedDestination_LocalFolder = false;
	}
}

void DicomViewPreferencePage::on_pushButton_PublicPACS_MedicalConnections_clicked()
{
	QString ip = "dicomserver.co.uk";
	int port = 104;
	QString AETitle = "server";
	if (ui.lineEdit_IP->text() != ip)
		ui.lineEdit_IP->setText(ip);
	if (ui.spinBox_Port->value() != port)
		ui.spinBox_Port->setValue(port);
	if (ui.lineEdit_HostAETitle->text() != AETitle)
		ui.lineEdit_HostAETitle->setText(AETitle);
}
void DicomViewPreferencePage::on_pushButton_PublicPACS_PixelMed_clicked()
{
	QString ip = "184.73.255.26";
	int port = 11112;
	QString AETitle = "AWSPIXELMEDPUB";
	if (ui.lineEdit_IP->text() != ip)
		ui.lineEdit_IP->setText(ip);
	if (ui.spinBox_Port->value() != port)
		ui.spinBox_Port->setValue(port);
	if (ui.lineEdit_HostAETitle->text() != AETitle)
		ui.lineEdit_HostAETitle->setText(AETitle);
}
void DicomViewPreferencePage::on_lineEdit_LocalFolderPath_textChanged(QString)
{
	QString directory = ui.lineEdit_LocalFolderPath->text();
	if (directory.isEmpty())
	{
		ui.lineEdit_LocalFolderPath->blockSignals(true);
		ui.lineEdit_LocalFolderPath->setText(NO_DIRECTORY_SPECIFIED);
		ui.lineEdit_LocalFolderPath->blockSignals(false);
	}
}
void DicomViewPreferencePage::on_pushButton_LocalFolderPath_clicked()
{
	QString directory = ui.lineEdit_LocalFolderPath->text();
	if (directory == NO_DIRECTORY_SPECIFIED)
		directory = "";
	QString new_directory = QFileDialog::getExistingDirectory(nullptr, tr("Set Local Storage Directory"), directory);
	if (directory == new_directory || new_directory.isEmpty() || new_directory == NO_DIRECTORY_SPECIFIED)
		return;
	if (new_directory.isEmpty())
		new_directory = NO_DIRECTORY_SPECIFIED;
	ui.lineEdit_LocalFolderPath->setText(new_directory);
}
void DicomViewPreferencePage::on_pushButton_SetTemporaryLocalFolderPath_clicked()
{
	ui.lineEdit_LocalFolderPath->setText("");
}
void DicomViewPreferencePage::on_buttonGroup_Protocol_buttonToggled(QAbstractButton*, bool)
{
	updateProtocolInUI();
}