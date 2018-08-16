#include "DicomView.h"
#include "inova_pacs_views_dicomview_Activator.h"

#include "ui_DicomViewControls.h"

#include <mitkIDataStorageService.h>
#include <mitkImage.h>

#include <ctkDICOMTableManager.h>
#include <ctkDICOMTableView.h>

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */   
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/oflog/oflog.h"
#include "dcmtk/dcmnet/scu.h"

#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryIQtStyleManager.h>
#include <berryQtPreferences.h>
#include <berryQtStyleManager.h>

#include <QDockWidget>
#include <QMessageBox>
#include <QTableView>
#include <QFileDialog>

#include <sstream>


#include <service/event/ctkEventConstants.h>
#include <ctkDictionary.h>
#include <mitkLogMacros.h>
#include <mitkDataNode.h>
#include <service/event/ctkEventAdmin.h>
#include <ctkServiceReference.h>
#include <mitkRenderingManager.h>
#include <QVector>
//#include <mitkContourModelSet.h>

#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMEnums.h>
#include <mitkDICOMTagsOfInterestHelper.h>
#include <mitkDICOMProperty.h>
//#include <mitkStringProperty.h>
#include <mitkPropertyNameHelper.h>

#include <mitkRTDoseReaderService.h>
#include <mitkRTPlanReaderService.h>
#include <mitkRTStructureSetReaderService.h>
//#include <mitkRTConstants.h>
#include <mitkIsoDoseLevelCollections.h>
//#include <mitkIsoDoseLevelSetProperty.h>
//#include <mitkIsoDoseLevelVectorProperty.h>
#include <mitkDoseImageVtkMapper2D.h>
#include <mitkRTUIConstants.h>
#include <mitkIsoLevelsGenerator.h>
#include <mitkDoseNodeHelper.h>

#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkRenderingModeProperty.h>
#include <mitkLocaleSwitch.h>
#include <mitkIOUtil.h>

//#include <ImporterUtil.h>
#include <array>


const string DicomView::VIEW_ID = "inova.pacs.views.dicomview";


void LocalSCU::notifyRECEIVEProgress(const unsigned long byteCount)
{
	DcmSCU::notifyRECEIVEProgress(byteCount);

	if (byteCount < prevByteCount)
		totalByteCount += byteCount;
	else
		totalByteCount += (byteCount - prevByteCount);
	prevByteCount = byteCount;

	stringstream ss;
	if (totalByteCount < 1024)
		ss << "Received " << totalByteCount << " bytes";
	else if (totalByteCount < 1024 * 1024)
		ss << "Received " << totalByteCount / 1024 << " KB";
	else if (totalByteCount < 100 * 1024 * 1024)
		ss << "Received " << setprecision(1) << std::fixed << float(totalByteCount) / 1024 / 1024 << " MB";
	else
		ss << "Received " << totalByteCount / 1024 / 1024 << " MB";

	emit progress_text(QString::fromStdString(ss.str()));
}
void LocalSCU::notifyInstanceStored(const OFString& /*filename*/, const OFString& /*sopClassUID*/, const OFString& /*sopInstanceUID*/) const
{
	auto pNum = const_cast<int*>(&num_received);
	(*pNum)++;
	emit progress(num_received); // number of series retrieved
}
void LocalSCU::cancel()
{
	T_ASC_PresentationContextID presID = findPresentationContextID(UID_MOVEStudyRootQueryRetrieveInformationModel, "" /* don't care about transfer syntax */);
	sendCANCELRequest(presID);
}


DicomView::DicomView()
	: m_Parent(nullptr)
	, ui(*new Ui::DicomViewControls)
{
}

DicomView::~DicomView()
{
	delete &ui;
}

// //! [DicomViewCreatePartControl]
void DicomView::CreateQtPartControl(QWidget *parent)
{
	/// Create GUI widgets.
	m_Parent = parent;
	ui.setupUi(parent);
	m_Parent->setEnabled(true);

	/// CTK signals.
	auto pluginContext = inova_pacs_views_dicomview_Activator::GetPluginContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	/// Creating an Event Publisher.
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		eventAdmin->publishSignal(this, SIGNAL(OpenDICOMSeries(const ctkDictionary&)), "data/OPENDICOMSERIES", Qt::DirectConnection);
	}

	/// Connections.
	connect(this->ui.pushButton_Query, SIGNAL(clicked()), this, SLOT(on_pushButton_Query_clicked()));
	connect(this->ui.pushButton_Retrieve, SIGNAL(clicked()), this, SLOT(on_pushButton_Retrieve_clicked()));
	//connect(this->ui.pushButton_Cancel, SIGNAL(clicked()), this, SLOT(on_canceled()));
	connect(this->ui.pushButton_AddToDataManager, SIGNAL(clicked()), this, SLOT(on_pushButton_AddToDataManager_clicked()));
	connect(this->ui.pushButton_ShowSettings, SIGNAL(clicked()), this, SLOT(on_pushButton_ShowSettings_clicked()));
	//connect(..., SIGNAL(run(const string&)), this, SLOT(on_pushButton_AddToDataManager_clicked()));
	// Connections for Settings
	connect(this->ui.lineEdit_IP, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_IP_textChanged(QString)));
	connect(this->ui.spinBox_Port, SIGNAL(valueChanged(int)), this, SLOT(on_spinBox_Port_valueChanged(int)));
	connect(this->ui.lineEdit_HostAETitle, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_HostAETitle_textChanged(QString)));
	connect(this->ui.pushButton_PublicPACS_MedicalConnections, SIGNAL(clicked()), this, SLOT(on_pushButton_PublicPACS_MedicalConnections_clicked()));
	connect(this->ui.pushButton_PublicPACS_PixelMed, SIGNAL(clicked()), this, SLOT(on_pushButton_PublicPACS_PixelMed_clicked()));
	//connect(this->ui.comboBox_Protocol, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_Protocol_currentIndexChanged(int)));
	connect(this->ui.buttonGroup_Protocol, SIGNAL(buttonToggled(QAbstractButton*, bool)), this, SLOT(on_buttonGroup_Protocol_buttonToggled(QAbstractButton*, bool)));
	connect(this->ui.buttonGroup_Destination, SIGNAL(buttonToggled(QAbstractButton*, bool)), this, SLOT(on_buttonGroup_Destination_buttonToggled(QAbstractButton*, bool)));
	connect(this->ui.lineEdit_DestinationAETitle, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_DestinationAETitle_textChanged(QString)));
	connect(this->ui.checkBox_AddToDataManager, SIGNAL(stateChanged(int)), this, SLOT(on_checkBox_AddToDataManager_stateChanged(int)));
	connect(this->ui.lineEdit_LocalFolderPath, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_LocalFolderPath_textChanged(QString)));
	connect(this->ui.pushButton_LocalFolderPath, SIGNAL(clicked()), this, SLOT(on_pushButton_LocalFolderPath_clicked()));
	connect(this->ui.pushButton_SetTemporaryLocalFolderPath, SIGNAL(clicked()), this, SLOT(on_pushButton_SetTemporaryLocalFolderPath_clicked()));
	connect(this->ui.lineEdit_LocalStorageIP, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_LocalStorageIP_textChanged(QString)));
	connect(this->ui.spinBox_LocalStoragePort, SIGNAL(valueChanged(int)), this, SLOT(on_spinBox_LocalStoragePort_valueChanged(int)));
	connect(this->ui.lineEdit_StorageAETitle, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_StorageAETitle_textChanged(QString)));
	// UI
	connect(this->ui.dicomTableManager->studiesTable(), SIGNAL(selectionChanged(const QStringList&)), this, SLOT(on_table_Studies_selectionChanged(const QStringList&)));
	connect(this->ui.dicomTableManager->seriesTable(), SIGNAL(selectionChanged(const QStringList&)), this, SLOT(on_table_Series_selectionChanged(const QStringList&)));

	/// Settings.
	updateSettings();

	/// UI customization.
	// Find tables
	array<ctkDICOMTableView*, 3> tables;
	tables[0] = ui.dicomTableManager->patientsTable();
	tables[1] = ui.dicomTableManager->studiesTable();
	tables[2] = ui.dicomTableManager->seriesTable();


	//auto context = inova_pacs_views_dicomview_Activator::GetPluginContext();
	//ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
	int sectionSize = 20;
	//QString stylesheet;
	//if (styleManagerRef)
	//{
	//	auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
	//	auto style_name = styleManager->GetStyle().name;
	//	stylesheet = styleManager->GetStylesheet();
	//}
	for (auto table : tables)
	{
		table->setTableSectionSize(sectionSize);
		//table->setStyleSheet(stylesheet);
	}
	//QIcon icon = style()->standardIcon(QStyle::SP_DialogOkButton);
	QString basePath = QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/");
	QIcon icon(berry::QtStyleManager::ThemeIcon(basePath + "go-next.svg"));
	QPixmap pixmap = icon.pixmap(QSize(24, 24));
	ui.label_Next_1->setPixmap(pixmap);
	ui.label_Next_2->setPixmap(pixmap);

	/// Settings visibility.
	are_settings_visible = false;
	updateSettingsVisibility();

	/// Update UI.
	is_retrieved = false;
	retrieveFolder = "";
	retrievedFiles.clear();
	//QSizePolicy sp_retain = ui.widget_Buttons->sizePolicy();
	//sp_retain.setRetainSizeWhenHidden(true);
	//ui.widget_Buttons->setSizePolicy(sp_retain);
	updateAddToDataManager();
	updateButtons();
}

void DicomView::updateSettings()
{
	is_savedDestination_LocalFolder = false;

	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	m_DicomViewPreferencesNode = prefService->GetSystemPreferences()->Node("/inova.pacs.views.dicomview");
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
void DicomView::updateProtocolInUI()
{
	bool is_CGET = ui.radioButton_Protocol_CGET->isChecked();
	int protocol = (is_CGET ? PROTOCOL_CGET : PROTOCOL_CMOVE);
	m_DicomViewPreferencesNode->PutInt("PACS protocol", protocol);

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
void DicomView::updateSettingsVisibility()
{
	QString basePath = QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/");
	QIcon icon(berry::QtStyleManager::ThemeIcon(basePath + (are_settings_visible ? "go-previous.svg" : "go-next.svg")));
	ui.pushButton_ShowSettings->setIcon(icon);
	ui.splitter_Settings->setVisible(are_settings_visible);
	ui.widget_SettingsSpace->setVisible(!are_settings_visible);
}
void DicomView::updateButtons()
{
	//if (retrievedFiles.size() > 0)
	//	ui.pushButton_AddToDataManager->setText("3. Add selected studies to Data Manager");
	//else
	//	ui.pushButton_AddToDataManager->setText("3. Add selected series to Data Manager");

	bool enabled = false;
	if (is_retrieved)
	{
		//QStringList selected = 
		//	(retrievedFiles.size() > 0) ? ui.dicomTableManager->studiesTable()->currentSelection()
		//	: ui.dicomTableManager->seriesTable()->currentSelection();
		QStringList selected = ui.dicomTableManager->seriesTable()->currentSelection();
		enabled = (selected.size() > 0);
	}
	ui.pushButton_AddToDataManager->setEnabled(enabled);
	ui.label_Next_2->setEnabled(enabled);

	//int num_studies = queriesByStudyUID.keys().size();
	//ui.pushButton_Retrieve->setEnabled(num_studies > 0);
}
void DicomView::updateAddToDataManager()
{
	bool add_to_DataManager = ui.checkBox_AddToDataManager->isChecked();
	ui.label_Next_2->setVisible(!add_to_DataManager);
	ui.pushButton_AddToDataManager->setVisible(!add_to_DataManager);
	//ui.widget_Buttons
	if (add_to_DataManager)
		ui.horizontalSpacer_Buttons->changeSize(1, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
	else
		ui.horizontalSpacer_Buttons->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
	ui.formLayout->update();// updateGeometry();
}
QString DicomView::getDestinationAE(RetrieveProtocol protocol)
{
	QString retval;
	if (protocol == PROTOCOL_CGET && ui.radioButton_DestinationLocalFolder->isChecked())
		retval = ui.lineEdit_StorageAETitle->text();
	else
		retval = ui.lineEdit_DestinationAETitle->text();

	if (!retval.isEmpty() && retval != NO_DIRECTORY_SPECIFIED)
		return retval;
	return "POPEAE";
}
void DicomView::addSeriesToDataManager(const QStringList& listOfFilesForSeries, const shared_ptr<QString> modality)
{
	if (!listOfFilesForSeries.isEmpty())
	{
		auto pluginContext = inova_pacs_views_dicomview_Activator::GetPluginContext();

	
		//for rt data, if the modality tag isn't defined or is "CT" the image is handled like before
		if (modality != nullptr
			&& (modality->compare("RTDOSE", Qt::CaseInsensitive) == 0
			|| modality->compare("RTSTRUCT", Qt::CaseInsensitive) == 0
			|| modality->compare("RTPLAN", Qt::CaseInsensitive) == 0))
		{
			if (modality->compare("RTDOSE", Qt::CaseInsensitive) == 0)
			{
				auto doseReader = mitk::RTDoseReaderService();
				doseReader.SetInput(listOfFilesForSeries.front().toStdString());
				//doseReader.SetInput(ImporterUtil::getUTF8String(listOfFilesForSeries.front()));
				vector<itk::SmartPointer<mitk::BaseData> > readerOutput = doseReader.Read();
				if (!readerOutput.empty())
				{
					mitk::Image::Pointer doseImage = dynamic_cast<mitk::Image*>(readerOutput.at(0).GetPointer());

					mitk::DataNode::Pointer doseImageNode = mitk::DataNode::New();
					doseImageNode->SetData(doseImage);
					doseImageNode->SetName("RTDose");

					if (doseImage != nullptr)
					{
						std::string sopUID;
						if (mitk::GetBackwardsCompatibleDICOMProperty(0x0008, 0x0016, "dicomseriesreader.SOPClassUID", doseImage->GetPropertyList(), sopUID))
						{
							doseImageNode->SetName(sopUID);
						}

						berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
						berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

						if (prefNode.IsNull())
						{
							mitkThrow() << "Error in preference interface. Cannot find preset node under given name. Name: " << prefNode->ToString().toStdString();
						}

						//set some specific colorwash and isoline properties
						bool showColorWashGlobal = prefNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_COLORWASH_ID.c_str(), true);
						bool showIsolinesGlobal = prefNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_ISOLINES_ID.c_str(), true);

						//Set reference dose property
						double referenceDose = prefNode->GetDouble(mitk::RTUIConstants::REFERENCE_DOSE_ID.c_str(), mitk::RTUIConstants::DEFAULT_REFERENCE_DOSE_VALUE);

						mitk::ConfigureNodeAsDoseNode(doseImageNode, mitk::GeneratIsoLevels_Virtuos(), referenceDose, showColorWashGlobal);

						ctkServiceReference serviceReference = pluginContext->getServiceReference<mitk::IDataStorageService>();
						mitk::IDataStorageService* storageService = pluginContext->getService<mitk::IDataStorageService>(serviceReference);
						mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

						dataStorage->Add(doseImageNode);

						mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
					}
				}//END DOSE
			}
			else if (modality->compare("RTSTRUCT", Qt::CaseInsensitive) == 0)
			{
				auto structReader = mitk::RTStructureSetReaderService();
				//structReader.SetInput(ImporterUtil::getUTF8String(listOfFilesForSeries.front()));
				structReader.SetInput(listOfFilesForSeries.front().toStdString());
				vector<itk::SmartPointer<mitk::BaseData> > readerOutput = structReader.Read();

				if (readerOutput.empty())
				{
					MITK_ERROR << "No structure sets were created" << endl;
				}
				else
				{
					vector<mitk::DataNode::Pointer> modelVector;

					ctkServiceReference serviceReference = pluginContext->getServiceReference<mitk::IDataStorageService>();
					mitk::IDataStorageService* storageService = pluginContext->getService<mitk::IDataStorageService>(serviceReference);
					mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

					for (const auto& aStruct : readerOutput)
					{
						mitk::ContourModelSet::Pointer countourModelSet = dynamic_cast<mitk::ContourModelSet*>(aStruct.GetPointer());

						mitk::DataNode::Pointer structNode = mitk::DataNode::New();
						structNode->SetData(countourModelSet);
						structNode->SetProperty("name", aStruct->GetProperty("name"));
						structNode->SetProperty("color", aStruct->GetProperty("contour.color"));
						structNode->SetProperty("contour.color", aStruct->GetProperty("contour.color"));
						structNode->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
						structNode->SetVisibility(true, mitk::BaseRenderer::GetInstance(
							mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")));
						structNode->SetVisibility(false, mitk::BaseRenderer::GetInstance(
							mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget2")));
						structNode->SetVisibility(false, mitk::BaseRenderer::GetInstance(
							mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3")));
						structNode->SetVisibility(true, mitk::BaseRenderer::GetInstance(
							mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4")));

						dataStorage->Add(structNode);
					}
					mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
				}
			}
			else if (modality->compare("RTPLAN", Qt::CaseInsensitive) == 0)
			{
				auto planReader = mitk::RTPlanReaderService();
				//planReader.SetInput(ImporterUtil::getUTF8String(listOfFilesForSeries.front()));
				planReader.SetInput(listOfFilesForSeries.front().toStdString());
				vector<itk::SmartPointer<mitk::BaseData> > readerOutput = planReader.Read();
				if (!readerOutput.empty())
				{
					//there is no image, only the properties are interesting
					mitk::Image::Pointer planDummyImage = dynamic_cast<mitk::Image*>(readerOutput.at(0).GetPointer());

					mitk::DataNode::Pointer planImageNode = mitk::DataNode::New();
					planImageNode->SetData(planDummyImage);
					planImageNode->SetName("RTPlan");

					ctkServiceReference serviceReference = pluginContext->getServiceReference<mitk::IDataStorageService>();
					mitk::IDataStorageService* storageService = pluginContext->getService<mitk::IDataStorageService>(serviceReference);
					mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

					dataStorage->Add(planImageNode);
				}
			}
		}
		else
		{
			mitk::StringList seriesToLoad;
			QStringListIterator it(listOfFilesForSeries);

			while (it.hasNext())
			{
				seriesToLoad.push_back(it.next().toStdString());
				//seriesToLoad.push_back(ImporterUtil::getUTF8String(it.next()));
			}

			//Get Reference for default data storage.
			ctkServiceReference serviceReference = pluginContext->getServiceReference<mitk::IDataStorageService>();
			mitk::IDataStorageService* storageService = pluginContext->getService<mitk::IDataStorageService>(serviceReference);
			mitk::DataStorage* dataStorage = storageService->GetDefaultDataStorage().GetPointer()->GetDataStorage();

			std::vector<mitk::BaseData::Pointer> baseDatas = mitk::IOUtil::Load(seriesToLoad.front());
			for (const auto &data : baseDatas)
			{
				mitk::DataNode::Pointer node = mitk::DataNode::New();
				node->SetData(data);

				std::string nodeName = "Unnamed Dicom";

				std::string studyUID = "";
				std::string seriesUID = "";

				data->GetPropertyList()->GetStringProperty("DICOM.0020.000D", studyUID);
				data->GetPropertyList()->GetStringProperty("DICOM.0020.000E", seriesUID);

				if (!studyUID.empty())
				{
					nodeName = studyUID;
				}

				if (!seriesUID.empty())
				{
					if (!studyUID.empty())
					{
						nodeName += "/";
					}
					nodeName += seriesUID;
				}

				dataStorage->Add(node);
			}
		}
	}
	else
	{
		MITK_INFO << "There are no files for the current series";
	}
}

void DicomView::OnPreferencesChanged(const berry::IBerryPreferences*)
{
	updateSettings();
}

/*
static Uint8 findUncompressedPC(const OFString& sopClass, DcmSCU& scu)
{
	Uint8 pc;
	pc = scu.findPresentationContextID(sopClass, UID_LittleEndianExplicitTransferSyntax);
	if (pc == 0)
		pc = scu.findPresentationContextID(sopClass, UID_BigEndianExplicitTransferSyntax);
	if (pc == 0)
		pc = scu.findPresentationContextID(sopClass, UID_LittleEndianImplicitTransferSyntax);
	return pc;
}*/
void DicomView::on_pushButton_Query_clicked()
{
	/*LocalSCU scu;
	auto protocol = ui.radioButton_Protocol_CGET->isChecked() ? PROTOCOL_CGET : PROTOCOL_CMOVE;
	QString AETitle = getDestinationAE(protocol);
	scu.setAETitle(AETitle.toStdString());
	scu.setPeerHostName(ui.lineEdit_IP->text().toStdString());
	scu.setPeerPort(ui.spinBox_Port->value());
	scu.setPeerAETitle(ui.lineEdit_HostAETitle->text().toStdString());
	// Use presentation context for FIND/MOVE in study root, propose all uncompressed transfer syntaxes 
	OFList<OFString> ts;
	ts.push_back(UID_LittleEndianExplicitTransferSyntax);
	ts.push_back(UID_BigEndianExplicitTransferSyntax);
	ts.push_back(UID_LittleEndianImplicitTransferSyntax);
	scu.addPresentationContext(UID_FINDStudyRootQueryRetrieveInformationModel, ts);
	scu.addPresentationContext(UID_MOVEStudyRootQueryRetrieveInformationModel, ts);
	scu.addPresentationContext(UID_VerificationSOPClass, ts);

	// Initialize network
	OFCondition result = scu.initNetwork();
	if (result.bad())
	{
		DCMNET_ERROR("Unable to set up the network: " << result.text());
		return;
	}
	// Negotiate Association
	result = scu.negotiateAssociation();
	if (result.bad())
	{
		DCMNET_ERROR("Unable to negotiate association: " << result.text());
		return;
	}
	// Let's look whether the server is listening: Assemble and send C-ECHO request
	result = scu.sendECHORequest(0);
	if (result.bad())
	{
		DCMNET_ERROR("Could not process C-ECHO with the server: " << result.text());
		return;
	}
	// Assemble and send C-FIND request
	OFList<QRResponse*> findResponses;
	DcmDataset req;
	req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
	req.putAndInsertOFStringArray(DCM_StudyInstanceUID, "");
	T_ASC_PresentationContextID presID = findUncompressedPC(UID_FINDStudyRootQueryRetrieveInformationModel, scu);
	if (presID == 0)
	{
		DCMNET_ERROR("There is no uncompressed presentation context for Study Root FIND");
		return;
	}
	result = scu.sendFINDRequest(presID, &req, &findResponses);
	if (result.bad())
		return;
	else
		DCMNET_INFO("There are " << findResponses.size() << " studies available");
	*/

	try
	{
		queryResultDatabase.openDatabase(":memory:", "QUERY-DB");
	}
	catch (std::exception e)
	{
		MITK_ERROR << "Database error: " << queryResultDatabase.lastError();
		queryResultDatabase.closeDatabase();
		return;
	}
	queriesByStudyUID.clear();//?? or has to be saved to use different PACS?
	//updateUI();
	//qApp->processEvents();
	// For each of the selected server nodes, send the query
	QString labelText = "Query DICOM servers";
	QString cancelButtonText = "Cancel";
	QProgressDialog progress(labelText, cancelButtonText, 0, 100, nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
	// We don't want the progress dialog to resize itself, so we bypass the label by creating our own
	QLabel* progressLabel = new QLabel(tr("Initialization..."));
	progress.setLabel(progressLabel);
	this->progressDialog = &progress;
	progress.setWindowModality(Qt::ApplicationModal);
	progress.setMinimumDuration(0);
	progress.setValue(0);
	progress.show();

	if (!progress.wasCanceled())
	{
		// create a query for the current server
		query = new ctkDICOMQuery;

		try
		{
			auto protocol = ui.radioButton_Protocol_CGET->isChecked() ? PROTOCOL_CGET : PROTOCOL_CMOVE;
			QString AETitle = getDestinationAE(protocol);
			query->setCallingAETitle(AETitle);
			query->setCalledAETitle(ui.lineEdit_HostAETitle->text());
			query->setHost(ui.lineEdit_IP->text());
			query->setPort(ui.spinBox_Port->value());
			query->setPreferCGET(protocol == PROTOCOL_CGET);
			// populate the query with the current search options
			query->setFilters(ui.dicomQueryWidget->parameters());

			connect(&progress, SIGNAL(canceled()), query, SLOT(cancel()));
			connect(query, SIGNAL(progress(QString)), progressLabel, SLOT(setText(QString)));
			connect(query, SIGNAL(progress(int)), this, SLOT(on_queryProgress_changed(int)));

			// run the query against the selected server and put results in database
			query->query(queryResultDatabase);

			disconnect(query, SIGNAL(progress(QString)), progressLabel, SLOT(setText(QString)));
			disconnect(query, SIGNAL(progress(int)), this, SLOT(on_queryProgress_changed(int)));
			disconnect(&progress, SIGNAL(canceled()), query, SLOT(cancel()));
		}
		catch (std::exception e)
		{
			MITK_ERROR << "Query error: " << ui.lineEdit_IP->text();
			progress.setLabelText("Query error: " + ui.lineEdit_IP->text());
			delete query;
			query = nullptr;
		}

		if (query != nullptr)
		{
			for (const QString& studyUID : query->studyInstanceUIDQueried())
			{
				queriesByStudyUID[studyUID] = query;
			}
		}
	}

	if (!progress.wasCanceled())
	{
		model.setDatabase(queryResultDatabase.database());
		ui.dicomTableManager->setDICOMDatabase(&(queryResultDatabase));
	}

	progress.setValue(progress.maximum());
	progressDialog = nullptr;

	is_retrieved = false;
	retrieveFolder = "";
	retrievedFiles.clear();
	updateButtons();
}
void DicomView::on_pushButton_Retrieve_clicked()
{
	if (!ui.pushButton_Retrieve->isEnabled())
		return;

	QStringList selectedStudiesUIDs = ui.dicomTableManager->currentStudiesSelection();
	auto selectedStudiesUIDs_size = selectedStudiesUIDs.size();
	if (selectedStudiesUIDs_size == 0)
	{
		QMessageBox::information(nullptr, tr("Query Retrieve"), tr("There are no selected studies to start the retrieval process"));
		return;
	}

	QString labelText = "Retrieve from DICOM servers";
	QString cancelButtonText = "Cancel";
	QProgressDialog progress(labelText, cancelButtonText, 0, 0, nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
	// We don't want the progress dialog to resize itself, so we bypass the label by creating our own
	QLabel* progressLabel = new QLabel(tr("Initialization..."));

	// for each of the selected server nodes, send the query
	progress.setLabel(progressLabel);
	this->progressDialog = &progress;
	progress.setWindowModality(Qt::ApplicationModal);
	progress.setMinimumDuration(0);
	progress.setValue(0);
	progress.setMaximum(0);
	progress.setAutoClose(false);
	progress.show();

	// Initialization for ctkDICOMRetrieve
	ctkDICOMRetrieve retrieve;
	// only start new association if connection parameters change
	retrieve.setKeepAssociationOpen(true);
	// do the retrieval for each series that is selected in the tree view
	int num_retrieved = 0;
	int num_processed = 0;
	int num_errors = 0;

	// Clear the folder and make a new one if needed
	bool is_destination_folder_needed = false;
	if (ui.radioButton_DestinationLocalFolder->isChecked())
	{
		for (const QString& studyUID : selectedStudiesUIDs)
		{
			ctkDICOMQuery* query = queriesByStudyUID[studyUID];
			auto protocol = query->preferCGET() ? PROTOCOL_CGET : PROTOCOL_CMOVE;
			if (protocol == PROTOCOL_CGET)
			{
				is_destination_folder_needed = true;
				break;
			}
		}
	}
	temp_dir = ui.radioButton_DestinationLocalFolder->isChecked() ? make_shared<QTemporaryDir>() : nullptr;
	retrieveFolder = "";
	retrievedFiles.clear();
	// Check if we need the destination folder
	list<QString> existing_files;
	if (is_destination_folder_needed)
	{
		// Set folder
		retrieveFolder = ui.lineEdit_LocalFolderPath->text();
		if (retrieveFolder.isEmpty() || retrieveFolder == NO_DIRECTORY_SPECIFIED)
		{
			if (temp_dir && temp_dir->isValid())
			{
				retrieveFolder = temp_dir->path();
			}
			else
			{
				MITK_INFO << "Unable to create a temporary folder.";
				return;
			}
		}
		// Store the existing file names
		QDir dir(retrieveFolder);
		dir.setFilter(QDir::Files);
		existing_files = dir.entryList().toStdList();
	}

	int num_studies = selectedStudiesUIDs.size();
	for (const QString& studyUID : selectedStudiesUIDs)
	{
		num_processed++;
		//if (num_errors > 9)
		//{
		//	MITK_INFO << "Canceled. Too many errors to continue.";
		//	break;
		//}
		if (num_errors > 0 && (num_errors == 1 || num_errors % 10 == 0))
		{
			stringstream ss;
			if (num_errors == 1)
				ss << "Retrieve failed.";
			else
				ss << "Failed to retrieve " << num_errors << " queries.";
			ss << "  Keep trying ?";
			auto msg_res = QMessageBox::question(nullptr, tr("Query Retrieve"), QString::fromStdString(ss.str()), QMessageBox::Yes | QMessageBox::No);
			if (msg_res != QMessageBox::Yes)
				break;
		}
		MITK_INFO << studyUID.toUtf8().constData() << endl;
		if (progress.wasCanceled())
			break;
		progressLabel->setText(QString(tr("Retrieving:\n%1")).arg(studyUID));
		this->on_retrieveProgress_changed(0);

		// Get information which server we want to get the study from and prepare request accordingly
		ctkDICOMQuery* query = queriesByStudyUID[studyUID];

		// Initialization
		LocalSCU scu;
		auto protocol = query->preferCGET() ? PROTOCOL_CGET : PROTOCOL_CMOVE; //(ui.radioButton_Protocol_CGET->isChecked())
		QString AETitle = getDestinationAE(protocol);

		if (protocol == PROTOCOL_CGET) // (query->preferCGET()) //(ui.radioButton_Protocol_CGET->isChecked())
		{
			scu.setAETitle(AETitle.toStdString());
			scu.setPeerHostName(query->host().toStdString());
			scu.setPeerPort(query->port());
			scu.setPeerAETitle(query->calledAETitle().toStdString());
			// Use presentation context for FIND/MOVE in study root, propose all uncompressed transfer syntaxes   
			OFList<OFString> ts;
			ts.push_back(UID_LittleEndianExplicitTransferSyntax);
			ts.push_back(UID_BigEndianExplicitTransferSyntax);
			ts.push_back(UID_LittleEndianImplicitTransferSyntax);
			scu.addPresentationContext(UID_FINDStudyRootQueryRetrieveInformationModel, ts);
			scu.addPresentationContext(UID_MOVEStudyRootQueryRetrieveInformationModel, ts);
			scu.addPresentationContext(UID_VerificationSOPClass, ts);
			for (Uint16 i = 0; i < numberOfDcmLongSCUStorageSOPClassUIDs; i++)
			{
				scu.addPresentationContext(dcmLongSCUStorageSOPClassUIDs[i], ts, ASC_SC_ROLE_SCP);
			}
			// Initialize network
			OFCondition result = scu.initNetwork();
			if (result.bad())
			{
				DCMNET_ERROR("Unable to set up the network: " << result.text());
				num_errors++;
				continue;
			}
			// Negotiate Association
			result = scu.negotiateAssociation();
			if (result.bad())
			{
				DCMNET_ERROR("Unable to negotiate association: " << result.text());
				num_errors++;
				continue;
			}
			// Let's look whether the server is listening: Assemble and send C-ECHO request
			result = scu.sendECHORequest(0);
			if (result.bad())
			{
				DCMNET_ERROR("Could not process C-ECHO with the server: " << result.text());
				num_errors++;
				continue;
			}

			MITK_INFO << "About to retrieve " << studyUID << " from " << queriesByStudyUID[studyUID]->host();
			MITK_INFO << "Starting to retrieve using C-GET";

			T_ASC_PresentationContextID presID = scu.findPresentationContextID(UID_MOVEStudyRootQueryRetrieveInformationModel, "" /* don't care about transfer syntax */);
			if (presID == 0)
			{
				MITK_INFO << "Query Retrieve Request failed: No valid Study Root Presentation Context available";
				num_errors++;
				continue;
			}
			// Assemble and send C-MOVE request, for each study identified above
			//auto presID = findUncompressedPC(UID_MOVEStudyRootQueryRetrieveInformationModel, scu);
			//if (presID == 0)
			//{
			//	DCMNET_ERROR("There is no uncompressed presentation context for Study Root MOVE");
			//	return;
			//}
			DcmDataset req;
			req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
			req.putAndInsertOFStringArray(DCM_StudyInstanceUID, studyUID.toStdString());

			// fetches all images of this particular study
			list<RetrieveResponse*> responses;
			list<QString> existing_files;
			if (ui.radioButton_DestinationLocalFolder->isChecked())
			{
				scu.setStorageDir(retrieveFolder.toStdString());
			}

			connect(&progress, SIGNAL(canceled()), &scu, SLOT(cancel()));
			auto lambda = [this, progressLabel](QString value) { on_retrieveProgress_changed(progressLabel, value); };
			connect(&scu, &LocalSCU::progress_text, this, lambda);
			connect(&scu, SIGNAL(progress(int)), this, SLOT(on_retrieveProgress_changed(int)));

			result = scu.sendCGETRequest(presID, &req, &responses /* we are not interested into responses*/);
			//result = scu.sendMOVERequest(presID, query->callingAETitle().toStdString(), &req, &responses /* we are not interested into responses*/);
			if (result.good())
				MITK_INFO << "Received study: " << studyUID;
			if (responses.begin() == responses.end())
			{
				MITK_INFO << "No responses received at all! (at least one empty response always expected)";
				num_errors++;
				continue;
			}
			num_retrieved += responses.size(); //?? - 1;

			// Release association
			//scu.closeAssociation(DCMSCU_RELEASE_ASSOCIATION);

			disconnect(&scu, SIGNAL(progress(QString)), this, SLOT(on_retrieveProgress_changed(QString)));
			disconnect(&scu, &LocalSCU::progress_text, this, 0);
			//disconnect(&scu, SIGNAL(progress(int)), this, SLOT(on_retrieveProgress_changed(int)));
			disconnect(&progress, SIGNAL(canceled()), &scu, SLOT(cancel()));

			MITK_INFO << "Retrieve success";
		}
		else // protocol == PROTOCOL_CMOVE
		{
			// Create a database
			if (retrieveDatabase == nullptr)
				retrieveDatabase = QSharedPointer<ctkDICOMDatabase>(new ctkDICOMDatabase);
			try
			{
				retrieveDatabase->openDatabase(":memory:", "RETRIEVE-DB");
				//retrieveDatabase->openDatabase("C:\\o\\db.sql", "RETRIEVE-DB");
			}
			catch (std::exception e)
			{
				MITK_ERROR << "Database error: " << retrieveDatabase->lastError();
				retrieveDatabase->closeDatabase();
				num_errors++;
				continue;
			}
			retrieve.setDatabase(retrieveDatabase);
			// 
			retrieve.setCallingAETitle(AETitle); // query->callingAETitle());
			retrieve.setCalledAETitle(query->calledAETitle());
			retrieve.setPort(query->port());
			retrieve.setHost(query->host());
			retrieve.setMoveDestinationAETitle(AETitle); // query->callingAETitle());
			// TODO: check the model item to see if it is checked for now, assume all studies queried and shown to the user will be retrieved

			MITK_INFO << "About to retrieve " << studyUID << " from " << queriesByStudyUID[studyUID]->host();
			MITK_INFO << "Starting to retrieve using C-MOVE";
			connect(&progress, SIGNAL(canceled()), &retrieve, SLOT(cancel()));
			connect(&retrieve, SIGNAL(progress(QString)), progressLabel, SLOT(setText(QString)));
			connect(&retrieve, SIGNAL(progress(int)), this, SLOT(on_retrieveProgress_changed(int)));
			try
			{
				// perform the retrieve
				bool res;
				//if (query->preferCGET())
				//	res = retrieve.getStudy(studyUID);
				//else
				res = retrieve.moveStudy(studyUID);
				if (res)
					num_retrieved++;
			}
			catch (std::exception e)
			{
				MITK_ERROR << "Retrieve failed";
				num_errors++;
				continue;
			}

			disconnect(&retrieve, SIGNAL(progress(QString)), progressLabel, SLOT(setText(QString)));
			disconnect(&retrieve, SIGNAL(progress(int)), this, SLOT(on_retrieveProgress_changed(int)));
			disconnect(&progress, SIGNAL(canceled()), &retrieve, SLOT(cancel()));
			MITK_INFO << "Retrieve success";
		}
		
		int progress_value = 100 * num_processed / num_studies;
		progressDialog->setValue(progress_value);
		QApplication::processEvents();
	}

	// Check the files stored
	QDir dir(retrieveFolder);
	dir.setFilter(QDir::Files);
	auto files = dir.entryList().toStdList();
	// Find the difference in files in the folder (before they were stored and after)
	existing_files.sort();
	files.sort();
	retrievedFiles.clear();
	set_difference(files.begin(), files.end(), existing_files.begin(), existing_files.end(), inserter(retrievedFiles, retrievedFiles.begin()));
	for (auto& file : retrievedFiles)
	{
		QString path = dir.filePath(file);
		QString extension = QFileInfo(path).suffix();
		if (extension != "dcm" && extension != "dicom")
		{
			QFile::rename(path, path + ".dcm");
			file = file + ".dcm";
		}
	}

	this->is_retrieved = num_retrieved > 0 || retrievedFiles.size() > 0;
	/*if (!this->is_retrieved && !retrieveFolder.isEmpty())
	{
		// Count files in the destination folder
		QDir dir(retrieveFolder);
		dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		int total_files = dir.count();
		this->is_retrieved = (total_files > 0);
	}*/
	this->progressDialog = nullptr;

	updateButtons();

	on_pushButton_AddToDataManager_clicked();
}
void DicomView::on_pushButton_AddToDataManager_clicked()
{
	if (!ui.pushButton_AddToDataManager->isEnabled())
		return;

	QDir dir(retrieveFolder);
	int total_files = 0;
	if (!retrieveFolder.isEmpty() && dir.exists())
	{
		// Count files in the destination folder
		dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		total_files = dir.count();
	}
	if (retrieveDatabase.isNull() && total_files == 0)
	{
		MITK_INFO << "There is nothing to add to Data Manager.";
		return;
	}

	QStringList selectedSeriesUIDs = ui.dicomTableManager->currentSeriesSelection();
	if (selectedSeriesUIDs.size() == 0)
	{
		QMessageBox::information(nullptr, tr("Query Retrieve"), tr("There are no selected series to add to Data Manager"));
		return;
	}

	if (!retrieveDatabase.isNull())
	{
		MITK_INFO << "Adding the specified series...";
		for (const QString& seriesUID : selectedSeriesUIDs)
		{
			QStringList filesForSeries = retrieveDatabase->filesForSeries(seriesUID);
			shared_ptr<QString> pModality = nullptr;
			if (!filesForSeries.isEmpty())
			{
				QString modality = retrieveDatabase->fileValue(filesForSeries.at(0), "0008,0060");
				pModality = make_shared<QString>(modality);
			}
			addSeriesToDataManager(filesForSeries, pModality);
		}
	}
	if (total_files > 0)
	{
		MITK_INFO << "Finding the specified series among the retrieved files...";
		//auto files = dir.entryInfoList();
		//for (auto& file_info : files)
		//{
		//	QString file = file_info.filePath();
		//}
		QStringList files;
		for (auto& file : retrievedFiles)
		{
			files.push_back(dir.filePath(file));
		}
		ctkDictionary properties;
		properties["files"] = files;
		properties["series"] = ui.dicomTableManager->currentSeriesSelection();
		emit OpenDICOMSeries(properties);
	}
}
//void DicomView::on_UploadToPACSAction_run(const string& path)
//{}

void DicomView::on_canceled()
{
	//emit canceled();

	return;




	//!! Test code
//#include "ctkDICOMTester.h"
	/*ctkDICOMTester tester;
	//dicom.setDCMQRSCPPort(11112);
	//auto process = tester.startDCMQRSCP();
	//bool isStored = tester.storeData(QStringList() << "C:\\Data\\temp\\comb\\ct01+0006,50_004.dcm");
	//bool isOK = tester.stopDCMQRSCP();

	QString DCMQRSCPExecutable = "C:\\o\\m\\ep\\bin\\dcmprscp.exe";
	QString DCMQRSCPConfigFile = "C:\\o\\dcmqrscp\\dcmqrscp.cfg";
	tester.setDCMQRSCPExecutable(DCMQRSCPExecutable);
	if (tester.dcmqrscpExecutable() != DCMQRSCPExecutable)
	{
		std::cerr << __LINE__
			<< ": Failed to set dcmqrscp: " << qPrintable(DCMQRSCPExecutable)
			<< " value: " << qPrintable(tester.dcmqrscpExecutable())
			<< endl;
		return;
	}
	tester.setDCMQRSCPConfigFile(DCMQRSCPConfigFile);
	if (tester.dcmqrscpConfigFile() != DCMQRSCPConfigFile)
	{
		std::cerr << __LINE__
			<< ": Failed to set dcmqrscp config file: " << qPrintable(DCMQRSCPConfigFile)
			<< " value: " << qPrintable(tester.dcmqrscpConfigFile())
			<< endl;
		return;
	}

	QString& dcmqrscp = DCMQRSCPExecutable; //(tester.dcmqrscpExecutable());
	QString& dcmqrscpConf = DCMQRSCPConfigFile; //(tester.dcmqrscpConfigFile());

	if (!QFileInfo(dcmqrscp).exists() ||
		!QFileInfo(dcmqrscp).isExecutable())
	{
		std::cerr << __LINE__ << ": Wrong dcmqrscp executable: " << qPrintable(dcmqrscp) << endl;
	}

	if (!QFileInfo(dcmqrscpConf).exists() ||
		!QFileInfo(dcmqrscpConf).isReadable())
	{
		std::cerr << __LINE__ << ": Wrong dcmqrscp executable: " << qPrintable(dcmqrscp) << endl;
	}

	QProcess* process = tester.startDCMQRSCP();
	if (!process)
	{
		std::cerr << __LINE__ << ": Failed to start dcmqrscp: " << qPrintable(dcmqrscp)
			<< " with config file: " << qPrintable(dcmqrscpConf) << endl;
		return;
	}
	QStringList data;
	data << "C:\\Data\\temp\\comb\\ct01+0006,50_004.dcm";
	bool res = tester.storeData(data);
	if (!res)
	{
		std::cerr << __LINE__ << ": Failed to store data \"" << qPrintable(data.front()) 
			<< "\" via dcmqrscp: " << qPrintable(dcmqrscp)
			<< " with config file: " << qPrintable(dcmqrscpConf) << endl;
	}
	res = tester.stopDCMQRSCP();
	if (!res)
	{
		std::cerr << __LINE__ << ": Failed to stop dcmqrscp: " << qPrintable(dcmqrscp)
			<< " with config file: " << qPrintable(dcmqrscpConf) << endl;
		return;
	}*/
}
void DicomView::on_queryProgress_changed(int value)
{
	if (progressDialog == nullptr)
	{
		return;
	}
	if (query && progressDialog->wasCanceled())
	{
		query->cancel();
	}
	/*if (progressDialog->width() != 500)
	{
		QPoint pp = this->mapToGlobal(QPoint(0, 0));
		pp = QPoint(pp.x() + (this->width() - d->ProgressDialog->width()) / 2,
			pp.y() + (this->height() - d->ProgressDialog->height()) / 2);
		progressDialog->move(pp - QPoint((500 - d->ProgressDialog->width()) / 2, 0));
		progressDialog->resize(500, d->ProgressDialog->height());
	}*/
	float serverProgress = 100.0f;
	progressDialog->setValue((value / 101.0f) * serverProgress);
	QApplication::processEvents();
}
void DicomView::on_retrieveProgress_changed(int)
{
	if (progressDialog == 0)
	{
		return;
	}
	/*static int targetWidth = 700;
	if (d->ProgressDialog->width() != targetWidth)
	{
		QPoint pp = this->mapToGlobal(QPoint(0, 0));
		pp = QPoint(pp.x() + (this->width() - d->ProgressDialog->width()) / 2,
			pp.y() + (this->height() - d->ProgressDialog->height()) / 2);
		d->ProgressDialog->move(pp - QPoint((targetWidth - d->ProgressDialog->width()) / 2, 0));
		d->ProgressDialog->resize(targetWidth, d->ProgressDialog->height());
	}*/
	//progressDialog->setValue(value);
	//MITK_INFO << QString("setting value to %1").arg(value);
	QApplication::processEvents();
}
void DicomView::on_retrieveProgress_changed(QLabel* progressLabel, QString value)
{
	progressLabel->setText(value);
	QApplication::processEvents();
}

void DicomView::on_pushButton_ShowSettings_clicked()
{
	are_settings_visible = !are_settings_visible;
	updateSettingsVisibility();
}

void DicomView::on_lineEdit_IP_textChanged(QString)
{
	m_DicomViewPreferencesNode->Put("PACS host IP", ui.lineEdit_IP->text());
}
void DicomView::on_spinBox_Port_valueChanged(int)
{
	m_DicomViewPreferencesNode->PutInt("PACS host port", ui.spinBox_Port->value());
}
void DicomView::on_lineEdit_HostAETitle_textChanged(QString)
{
	m_DicomViewPreferencesNode->Put("PACS host AETitle", ui.lineEdit_HostAETitle->text());
}
void DicomView::on_pushButton_PublicPACS_MedicalConnections_clicked()
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
void DicomView::on_pushButton_PublicPACS_PixelMed_clicked()
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
void DicomView::on_buttonGroup_Protocol_buttonToggled(QAbstractButton*, bool)
{
	updateProtocolInUI();
}
void DicomView::on_buttonGroup_Destination_buttonToggled(QAbstractButton*, bool)
{
	int destination = (ui.radioButton_DestinationLocalFolder->isChecked() ? DESTINATION_LOCALFOLDER : DESTINATION_PACS);

	if (is_savedDestination_LocalFolder && destination == DESTINATION_PACS)
		return;

	m_DicomViewPreferencesNode->PutInt("PACS destination", destination);
}
void DicomView::on_lineEdit_DestinationAETitle_textChanged(QString)
{
	m_DicomViewPreferencesNode->Put("PACS destination AETitle", ui.lineEdit_DestinationAETitle->text());
}
void DicomView::on_checkBox_AddToDataManager_stateChanged(int)
{
	m_DicomViewPreferencesNode->PutBool("add to DataManager", ui.checkBox_AddToDataManager->isChecked());
	updateAddToDataManager();
}
void DicomView::on_lineEdit_LocalFolderPath_textChanged(QString)
{
	QString directory = ui.lineEdit_LocalFolderPath->text();
	if (directory.isEmpty())
	{
		ui.lineEdit_LocalFolderPath->blockSignals(true);
		ui.lineEdit_LocalFolderPath->setText(NO_DIRECTORY_SPECIFIED);
		ui.lineEdit_LocalFolderPath->blockSignals(false);
	}
	if (directory == NO_DIRECTORY_SPECIFIED)
		directory = "";
	m_DicomViewPreferencesNode->Put("PACS storage local folder", directory);
}
void DicomView::on_pushButton_LocalFolderPath_clicked()
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
void DicomView::on_pushButton_SetTemporaryLocalFolderPath_clicked()
{
	ui.lineEdit_LocalFolderPath->setText("");
}
void DicomView::on_lineEdit_LocalStorageIP_textChanged(QString)
{
	m_DicomViewPreferencesNode->Put("PACS storage local IP", ui.lineEdit_LocalStorageIP->text());
}
void DicomView::on_spinBox_LocalStoragePort_valueChanged(int)
{
	m_DicomViewPreferencesNode->PutInt("PACS storage local port", ui.spinBox_LocalStoragePort->value());
}
void DicomView::on_lineEdit_StorageAETitle_textChanged(QString /*value*/)
{
	m_DicomViewPreferencesNode->Put("PACS storage local AETitle", ui.lineEdit_StorageAETitle->text());
}

void DicomView::on_table_Studies_selectionChanged(const QStringList& selected)
{
	ui.pushButton_Retrieve->setEnabled(selected.size() > 0);
	ui.label_Next_1->setEnabled(selected.size() > 0);
	//updateButtons();
}
void DicomView::on_table_Series_selectionChanged(const QStringList& /*selected*/)
{
	updateButtons();
}
