#include "DicomView.h"
#include "my_pacs_views_dicomview_Activator.h"

#include <mitkIDataStorageService.h>
#include <mitkImage.h>

#include <ctkDICOMTableManager.h>

#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <QDockWidget>
#include <QMessageBox>

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



const string DicomView::VIEW_ID = "my.pacs.views.dicomview";

DicomView::DicomView() : m_Parent(nullptr)
{
}

DicomView::~DicomView()
{
}

// //! [DicomViewCreatePartControl]
void DicomView::CreateQtPartControl(QWidget *parent)
{
  /// Create GUI widgets.
  m_Parent = parent;
  ui.setupUi(parent);
  m_Parent->setEnabled(true);

  /// Settings.
  updateSettings();

  /// Connections.
  connect(this->ui.pushButton_Query, SIGNAL(clicked()), this, SLOT(on_pushButton_Query_clicked()));
  connect(this->ui.pushButton_Retrieve, SIGNAL(clicked()), this, SLOT(on_pushButton_Retrieve_clicked()));
  connect(this->ui.pushButton_Cancel, SIGNAL(clicked()), this, SLOT(on_canceled()));
  connect(this->ui.pushButton_AddToDataManager, SIGNAL(clicked()), this, SLOT(on_pushButton_AddToDataManager_clicked()));
  //connect(..., SIGNAL(run(const string&)), this, SLOT(on_pushButton_AddToDataManager_clicked()));
  // Connections for Settings
  connect(this->ui.lineEdit_IP, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_IP_textChanged(QString)));
  connect(this->ui.spinBox_Port, SIGNAL(valueChanged(int)), this, SLOT(on_spinBox_Port_valueChanged(int)));
  connect(this->ui.lineEdit_AETitle, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_AETitle_textChanged(QString)));
  connect(this->ui.comboBox_Protocol, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_Protocol_currentIndexChanged(int)));
  connect(this->ui.lineEdit_StorageAETitle, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_StorageAETitle_textChanged(QString)));

}

void DicomView::updateSettings()
{
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	m_DicomViewPreferencesNode = prefService->GetSystemPreferences()->Node("/my.pacs.views.dicomview");
	ui.lineEdit_IP->blockSignals(true);
	ui.spinBox_Port->blockSignals(true);
	ui.lineEdit_AETitle->blockSignals(true);
	ui.comboBox_Protocol->blockSignals(true);
	ui.lineEdit_StorageAETitle->blockSignals(true);
	ui.lineEdit_IP->setText(m_DicomViewPreferencesNode->Get("PACS IP", "127.0.0.1"));
	ui.spinBox_Port->setValue(m_DicomViewPreferencesNode->GetInt("PACS port", 11112));
	ui.lineEdit_AETitle->setText(m_DicomViewPreferencesNode->Get("PACS AETitle", "SERVERAE"));
	ui.comboBox_Protocol->setCurrentIndex(m_DicomViewPreferencesNode->GetInt("PACS protocol", 0));
	ui.lineEdit_StorageAETitle->setText(m_DicomViewPreferencesNode->Get("PACS storage AETitle", "SERVERAE"));
	ui.lineEdit_IP->blockSignals(false);
	ui.spinBox_Port->blockSignals(false);
	ui.lineEdit_AETitle->blockSignals(false);
	ui.comboBox_Protocol->blockSignals(false);
	ui.lineEdit_StorageAETitle->blockSignals(false);
}
void DicomView::addSeriesToDataManager(const QStringList& listOfFilesForSeries, const shared_ptr<QString> modality)
{
	if (!listOfFilesForSeries.isEmpty())
	{
		auto pluginContext = my_pacs_views_dicomview_Activator::GetPluginContext();

	
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

void DicomView::SetFocus()
{
}

void DicomView::OnPreferencesChanged(const berry::IBerryPreferences*)
{
	updateSettings();
}

void DicomView::on_pushButton_Query_clicked()
{
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
	queriesByStudyUID.clear();
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
			query->setCallingAETitle(ui.lineEdit_StorageAETitle->text());
			query->setCalledAETitle(ui.lineEdit_AETitle->text());
			query->setHost(ui.lineEdit_IP->text());
			query->setPort(ui.spinBox_Port->value());
			query->setPreferCGET(ui.comboBox_Protocol->currentIndex() == PROTOCOL_CGET);

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

		for(const QString& studyUID : query->studyInstanceUIDQueried())
		{
			queriesByStudyUID[studyUID] = query;
		}
	}

	if (!progress.wasCanceled())
	{
		model.setDatabase(queryResultDatabase.database());
		ui.dicomTableManager->setDICOMDatabase(&(queryResultDatabase));
	}
	int num_studies = queriesByStudyUID.keys().size();
	ui.pushButton_Retrieve->setEnabled(num_studies > 0);

	progress.setValue(progress.maximum());
	progressDialog = nullptr;
	query = nullptr;
}
void DicomView::on_pushButton_Retrieve_clicked()
{
	if (!ui.pushButton_Retrieve->isEnabled())
		return;

	QStringList selectedStudiesUIDs = ui.dicomTableManager->currentStudiesSelection();
	if (selectedStudiesUIDs.size() == 0)
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

	ctkDICOMRetrieve* retrieve = new ctkDICOMRetrieve;
	// only start new association if connection parameters change
	retrieve->setKeepAssociationOpen(true);
	// pull from GUI
	//retrieve->setMoveDestinationAETitle(ui.lineEdit_AETitle->text());

	// do the retrieval for each series that is selected in the tree view
	int num_retrieved = 0;
	for(const QString& studyUID : selectedStudiesUIDs)
	{
		MITK_INFO << studyUID.toUtf8().constData() << endl;
		if (progress.wasCanceled())
		{
			break;
		}
		progressLabel->setText(QString(tr("Retrieving:\n%1")).arg(studyUID));
		this->on_retrieveProgress_changed(0);

		// Get information which server we want to get the study from and prepare request accordingly
		ctkDICOMQuery* query = queriesByStudyUID[studyUID];
		///////////////////////////////////////////////////////////////
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
			return;
		}
		///////////////////////////////////////////////////////////////
		retrieve->setDatabase(retrieveDatabase);
		retrieve->setCallingAETitle(query->callingAETitle());
		retrieve->setCalledAETitle(query->calledAETitle());
		retrieve->setPort(query->port());
		retrieve->setHost(query->host());
		retrieve->setMoveDestinationAETitle(query->callingAETitle());
		// TODO: check the model item to see if it is checked for now, assume all studies queried and shown to the user will be retrieved
		MITK_INFO << "About to retrieve " << studyUID << " from " << queriesByStudyUID[studyUID]->host();
		MITK_INFO << "Starting to retrieve";

		connect(&progress, SIGNAL(canceled()), retrieve, SLOT(cancel()));
		connect(retrieve, SIGNAL(progress(QString)), progressLabel, SLOT(setText(QString)));
		connect(retrieve, SIGNAL(progress(int)), this, SLOT(on_retrieveProgress_changed(int)));
		try
		{
			// perform the retrieve
			bool res;
			if (query->preferCGET())
				res = retrieve->getStudy(studyUID);
			else
				res = retrieve->moveStudy(studyUID);
			if (res)
				num_retrieved++;
		}
		catch (std::exception e)
		{
			MITK_ERROR << "Retrieve failed";
			auto msg_res = QMessageBox::question(nullptr, tr("Query Retrieve"), tr("Retrieve failed.  Keep trying?"), QMessageBox::Yes | QMessageBox::No);
			if (msg_res == QMessageBox::Yes)
				continue;
			else
				break;
		}

		disconnect(retrieve, SIGNAL(progress(QString)), progressLabel, SLOT(setText(QString)));
		disconnect(retrieve, SIGNAL(progress(int)), this, SLOT(on_retrieveProgress_changed(int)));
		disconnect(&progress, SIGNAL(canceled()), retrieve, SLOT(cancel()));
		MITK_INFO << "Retrieve success";
	}

	//QString message = (retrieve->wasCanceled()) ? tr("Retrieve Process Canceled") : tr("Retrieve Process Finished");
	//QMessageBox::information(nullptr, tr("Query Retrieve"), message);


	ui.pushButton_AddToDataManager->setEnabled(num_retrieved > 0);

	delete retrieve;
	progressDialog = nullptr;
}
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
void DicomView::on_pushButton_AddToDataManager_clicked()
{
	if (!ui.pushButton_AddToDataManager->isEnabled())
		return;
	if (retrieveDatabase.isNull())
		return;

	QStringList selectedSeriesUIDs = ui.dicomTableManager->currentSeriesSelection();
	if (selectedSeriesUIDs.size() == 0)
	{
		QMessageBox::information(nullptr, tr("Query Retrieve"), tr("There are no selected series to add to Data Manager"));
		return;
	}

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
//void DicomView::on_UploadToPACSAction_run(const string& path)
//{}

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
void DicomView::on_retrieveProgress_changed(int value)
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
	progressDialog->setValue(value);
	//MITK_INFO << QString("setting value to %1").arg(value);
	QApplication::processEvents();
}


void DicomView::on_lineEdit_IP_textChanged(QString value)
{
	m_DicomViewPreferencesNode->Put("PACS IP", ui.lineEdit_IP->text());
}
void DicomView::on_spinBox_Port_valueChanged(int value)
{
	m_DicomViewPreferencesNode->PutInt("PACS port", ui.spinBox_Port->value());
}
void DicomView::on_lineEdit_AETitle_textChanged(QString value)
{
	m_DicomViewPreferencesNode->Put("PACS AETitle", ui.lineEdit_AETitle->text());
}
void DicomView::on_comboBox_Protocol_currentIndexChanged(int value)
{
	m_DicomViewPreferencesNode->PutInt("PACS protocol", ui.comboBox_Protocol->currentIndex());
}
void DicomView::on_lineEdit_StorageAETitle_textChanged(QString value)
{
	m_DicomViewPreferencesNode->Put("PACS storage AETitle", ui.lineEdit_StorageAETitle->text());
}