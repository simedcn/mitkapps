#include "DataManager.h"
#include <itkGDCMImageIO.h>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <mitkIOUtil.h>
#include <itkImageFileReader.h>
#include <mitkRenderingManager.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include "my_awesomeproject_renderwindoweditor_Activator.h"
#include "PatientSelector.h"
#include <PopeElements.h>
#include <QCoreApplication>
#include <memory>
#include <map>
#include <vector>
#include <set>
#include <utility>

DataManager::DataManager(mitk::DataStorage *datastorage, QObject *parent) :
  QObject(parent),
  m_DataStorage(datastorage),
  m_Settings("savedFiles.ini",QSettings::IniFormat)
{
	/// CTK signals.
	auto pluginContext = my_awesomeproject_renderwindoweditor_Activator::GetPluginContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "data/OPENDICOMSERIES";
		eventAdmin->subscribeSlot(this, SLOT(on_Action_OpenDICOMSeries_clicked(ctkEvent)), propsForSlot);
	}
}

bool DataManager::ProbeDicomFile(const std::string& filename, std::string& patientId)
{
	itk::GDCMImageIO::Pointer reader = itk::GDCMImageIO::New();
	if(!reader->CanReadFile(filename.c_str()))
	return false;
	reader->SetFileName(filename);
	reader->ReadImageInformation();
	char id[512];
	reader->GetPatientID(id);
	patientId = id;
	return true;
}

void DataManager::AddImage(const std::string& name, mitk::Image::Pointer image)
{
  bool isInDS = false;//is the datanode already loaded?
  auto allNodes = this->m_DataStorage->GetAll();
  auto iterator = allNodes->Begin();
  while(iterator != allNodes->End())
  {
    mitk::DataNode::Pointer datanode = iterator->Value();
    ++iterator;
    if(datanode->GetName().compare(name) == 0)
    {
      isInDS = true;
      iterator = allNodes->End();
    }
  }

  mitk::DataNode::Pointer dn = mitk::DataNode::New();
  if(!isInDS)
  {
  dn->SetName(name);
  dn->SetData(image);
  this->m_DataStorage->Add(dn);
  }
}

int DataManager::LoadImageSet(const QString &filenameAndDirectory)
{
  std::string patientId;
  bool success = this->ProbeDicomFile(filenameAndDirectory.toStdString(), patientId);
  if(!success) return 0;

  bool addImage = m_PatientId.isEmpty() || (QString(patientId.c_str()) == m_PatientId);
  if(!addImage)
  {
    QMessageBox msgBox;
    msgBox.setText("The file you chose contains data of a different patient");
    msgBox.setInformativeText("Do you want to save your changes and clear the datastorage?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Save:
      this->SaveDataOfCurrentPatient();
      break;
    case QMessageBox::Cancel:
      return 0;
      break;
    default:
      break;
    }
    m_DataStorage->Remove(this->m_DataStorage->GetSubset(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)))));
  }

  m_PatientId = patientId.c_str();
  QFileInfo fi(filenameAndDirectory);
  this->m_CurrentPath = fi.dir().absolutePath();

  std::vector<std::string> loadedFiles = this->LoadDataOfCurrentPatient();
  auto loaded = mitk::IOUtil::Load(filenameAndDirectory.toStdString());

  for (auto baseData : loaded)
  {
    mitk::DataNode::Pointer dn = mitk::DataNode::New();
    std::string curPatID;
    if (baseData->GetProperty("dicom.patient.PatientID"))
      curPatID = baseData->GetProperty("dicom.patient.PatientID")->GetValueAsString();
    else if (baseData->GetProperty("DICOM.0010.0020"))
      curPatID = baseData->GetProperty("DICOM.0010.0020")->GetValueAsString();


    std::string imageName = "image";
    if(baseData->GetProperty("dicom.series.SeriesInstanceUID"))
      imageName = baseData->GetProperty("dicom.series.SeriesInstanceUID")->GetValueAsString();
    if(baseData->GetProperty("dicom.series.Modality"))
    {
      imageName.append("_");
      imageName.append(baseData->GetProperty("dicom.series.Modality")->GetValueAsString());
    }
    if(patientId.compare(curPatID) == 0)
    {
      bool isInDS = false;//is the datanode already loaded?
      isInDS = !loadedFiles.empty() && std::find(loadedFiles.begin(),loadedFiles.end(), imageName) == loadedFiles.end();

      mitk::Image* img = dynamic_cast<mitk::Image*>(baseData.GetPointer());
      if(!isInDS && img)
      {
        this->AddImage(imageName, img);
      }
    }
  }
  // Initialize views as axial, sagittal, coronar (from
  // top-left to bottom)
  mitk::TimeGeometry::ConstPointer geo = this->m_DataStorage->ComputeBoundingGeometry3D(this->m_DataStorage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);
  //this->InitializeView();

  return 0;
}

void DataManager::SaveDataOfCurrentPatient()
{
  auto allNodes = this->m_DataStorage->GetAll();
  auto iterator = allNodes->Begin();
  while(iterator != allNodes->End())
  {
    mitk::DataNode::Pointer datanode = iterator->Value();
    mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
    if(image)
    {

      QString filename = datanode->GetName().c_str();
      filename.append(".nrrd");
      QString path = this->m_CurrentPath+"/"+filename;
      mitk::IOUtil::Save(image,path.toStdString());
      this->m_Settings.beginGroup(this->m_PatientId);

      this->m_Settings.setValue(datanode->GetName().c_str(),path);
      this->m_Settings.endGroup();
    }
    ++iterator;
  }
}

std::vector<std::string> DataManager::LoadDataOfCurrentPatient()
{
  std::vector<std::string> loadedFiles;
  if(this->m_CurrentPath.isEmpty() || this->m_PatientId.isEmpty()) return loadedFiles;
  this->m_Settings.beginGroup(this->m_PatientId);
  auto keys = this->m_Settings.childKeys();
  for(auto key : keys)
  { /*dicom.series.SeriesInstanceUID*/
    QString segFile = this->m_Settings.value(key).toString();
	mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(segFile.toStdString());
    this->AddImage(key.toStdString(),image);
    loadedFiles.push_back(key.toStdString());
  }
  this->m_Settings.endGroup(); //patientId
  return loadedFiles;
}

void DataManager::OpenDICOMSeries(const ctkEvent& event)
{
	QStringList files = event.getProperty("files").toStringList();
	QStringList series = event.getProperty("series").toStringList();
	int error_code = 0;
	for (const auto& file : files)
	{
		if (error_code != 0)
		{
			QString text = "Do you want to continue reading the files retrieved and saved on the disk?";
			auto msg_res = QMessageBox::question(nullptr, tr("Add to Data Manager"), text, QMessageBox::Yes | QMessageBox::No);
			if (msg_res != QMessageBox::Yes)
				return;
		}
		std::string filepath = file.toStdString();
		QString seriesInstanceUID = Elements::get_seriesInstanceUID(filepath);
		if (seriesInstanceUID.isEmpty())
		{//??
		 //continue;
			MITK_INFO << "Unable to read seriesInstanceUID from the file \"" << filepath << "\". Solution: load all files from the folder.";
			QString dir_path = QFileInfo(file).absolutePath();
			error_code = LoadImageFolder(dir_path);
			return;
		}
		bool is_in_list = false;
		for (const auto& s : series)
		{
			if (s == seriesInstanceUID)
			{
				is_in_list = true;
				break;
			}
		}
		if (!is_in_list)
			continue;
		LoadImageDataSet(file);
	}
}
int DataManager::AskAboutNewPatient()
{
	QMessageBox msgBox;
	msgBox.setText("The data you selected refers to another patient.");
	msgBox.setInformativeText("Do you want to save your changes and clear the datastorage?");
	msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Save);
	int ret = msgBox.exec();
	switch (ret)
	{
	case QMessageBox::Save:
		this->SaveDataOfCurrentPatient();
		break;
	case QMessageBox::Cancel:
		return -2;
		break;
	default:
		break;
	}
	auto condition = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));
	m_DataStorage->Remove(this->m_DataStorage->GetSubset(condition));
	return 0;
}
int DataManager::LoadImageDataSet(const QString &filenameAndDirectory)
{
	if (filenameAndDirectory.size() == 0)
		return -9;

	/// Check the file if it is readable
	std::string filepath = filenameAndDirectory.toStdString();
	QString patientId = Elements::get_patientId_or_patientName(filepath);
	if (patientId.isEmpty())
		return -1;

	/// Check if the image is from the same patient
	bool toAsk = (!m_PatientId.isEmpty() && patientId != m_PatientId);
	if (toAsk)
	{
		int retval = AskAboutNewPatient();
		if (retval != 0)
			return retval;
	}

	/// Update PatientId, CurrentFolder, and WorkDirectory
	m_PatientId = patientId;
	QFileInfo fi(filenameAndDirectory);

	/// Load data
	std::vector<std::string> loadedFiles = this->LoadDataOfCurrentPatient();
	auto loaded = mitk::IOUtil::Load(filepath);

	bool is_first = true;
	for (const auto baseData : loaded)
	{
		//mitk::DataNode::Pointer dn = mitk::DataNode::New();
		/// Get patient ID
		std::string curPatID = Elements::get_patientId_or_patientName(baseData);
		if (patientId == QString::fromStdString(curPatID))
		{
			/// Generate an image name
			std::string image_name = Elements::get_imageName(baseData);
			//MITK_INFO << image_name;
			/// Detect if the datanode is already saved
			bool isInDS = !loadedFiles.empty() && (std::find(loadedFiles.begin(), loadedFiles.end(), image_name) != loadedFiles.end());
			/// Load if it is a new one
			mitk::Image* img = dynamic_cast<mitk::Image*>(baseData.GetPointer());
			if (!isInDS && img)
			{
				this->AddImage(image_name, img);
				/// Select the first node loaded
				if (is_first)
				{
					//datanode->SetSelected(true);


					is_first = false;
				}
			}
		}
	}
	/// Initialize views as axial, sagittal, coronar (from top-left to bottom)
	auto geo = this->m_DataStorage->ComputeBoundingGeometry3D(this->m_DataStorage->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews(geo);
	//this->InitializeView();

	return 0;
}
QStringList get_all_subdirs(QDir dir)
{
	QStringList subDirs;
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	subDirs = dir.entryList();
	int num_subfolders = subDirs.size();

	// Add absolute path
	for (auto& sub_dir : subDirs)
	{
		QDir subDir(dir.absolutePath());
		sub_dir = subDir.filePath(sub_dir);
	}

	// Add subfolders
	if (num_subfolders > 0)
	{
		for (const auto& sub_dir_relative_path : subDirs)
		{
			QDir subDir(dir.absolutePath());
			subDir = subDir.filePath(sub_dir_relative_path);
			auto folders = get_all_subdirs(subDir);
			subDirs.append(folders);
		}
	}
	return subDirs;
}
int DataManager::LoadImageFolder(const QString& directory)
{
	if (directory.size() == 0)
		return -19;

	/// Get all subfolders if needed
	std::list<QDir> dirs;
	QDir main_dir(directory);
	dirs.push_back(main_dir);
	main_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	QStringList subDirs = main_dir.entryList();
	if (subDirs.size() > 0)
	{
		QMessageBox msgBox;
		msgBox.setText("The folder you selected contains subfolders.");
		msgBox.setInformativeText("Do you want to run the search in these folders as well?");
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::Yes);
		int ret = msgBox.exec();
		if (ret == QMessageBox::Yes)
		{
			QStringList all_subdirs = get_all_subdirs(main_dir);
			for (auto& subdir : all_subdirs)
			{
				dirs.push_back(subdir);
			}
		}
	}

	/// Go through all the dirs
	std::vector<mitk::BaseData::Pointer> all_loaded;
	for (auto& dir : dirs)
	{
		/// Get all DCM files from the directory
		dir.setFilter(QDir::Files);
		dir.setNameFilters(QStringList("*.dcm"));
		auto files = dir.entryList();
		//string path = dir.absolutePath().toStdString();
		//int num = files.size();

		/// Find the first correct (readable) file and delete wrong files from the list
		QString correct_file;
		auto it = files.begin();
		while (it != files.end())
		{
			const auto& file = *it;
			QString path = dir.filePath(file);
			//MITK_INFO << path.toStdString();
			QString id = Elements::get_patientId_or_patientName(path);
			if (id.isEmpty())
			{
				it = files.erase(it);
			}
			else
			{
				correct_file = path;
				break;
			}
		}
		if (correct_file.isEmpty())
			continue;
		std::string filepath = correct_file.toStdString();

		/// Read data
		auto loaded = mitk::IOUtil::Load(filepath);
		if (loaded.size() == 0)
			return -11;

		all_loaded.insert(all_loaded.end(), loaded.begin(), loaded.end());
	}
	if (all_loaded.size() == 0)
		return -12;

	/// Group the data by patient id
	std::map<std::string, std::list<mitk::BaseData::Pointer>> groups;
	for (const auto baseData : all_loaded)
	{
		/// Get patient ID
		std::string patientID = Elements::get_patientId_or_patientName(baseData);
		groups[patientID].push_back(baseData);
	}

	/// Get all patient IDs
	std::vector<std::string> patientIDs(groups.size());
	std::vector<std::shared_ptr<PatientDescription>> patientDescriptors(groups.size());
	int i = 0;
	for (const auto patient : groups)
	{
		const std::string& id = patient.first;
		patientIDs[i] = id;
		QString str_id = QString::fromStdString(id);
		const auto& baseDataList = patient.second;
		std::string name = Elements::find_patientName(baseDataList);
		std::string patientName = Elements::recognize_name(name);
		QString str_name = QString::fromStdString(patientName);
		auto images = Elements::get_imageNames(baseDataList);
		patientDescriptors[i] = std::make_shared<PatientDescription>(str_id, str_name, images);
		i++;
	}

	/// Select the patient
	int index = 0;
	if (groups.size() > 1)
	{
		PatientSelector patientSelector(nullptr);
		patientSelector.SetPatientData(patientDescriptors);
		patientSelector.SetFolder(directory);
		int retval = patientSelector.exec();
		if (retval != QDialog::Accepted)
			return -13;
		index = patientSelector.SelectedPatientIDIndex();
		if (index < 0 || index >= patientIDs.size())
			return -14;
	}
	std::string selected_id = patientIDs[index];

	/// Check if the image is from the same patient
	bool toAsk = !m_PatientId.isEmpty() && (selected_id != m_PatientId.toStdString());
	if (toAsk)
	{
		int retval = AskAboutNewPatient();
		if (retval != 0)
			return retval;
	}

	/// Update PatientId, CurrentFolder, and WorkDirectory
	m_PatientId = QString::fromStdString(selected_id);

	/// Load data
	std::vector<std::string> loadedFiles = this->LoadDataOfCurrentPatient();
	for (const auto baseData : groups[selected_id])
	{
		/// Generate an image name
		std::string image_name = Elements::get_imageName(baseData);
		//MITK_INFO << image_name;
		/// Detect if the datanode is already saved
		bool isInDS = !loadedFiles.empty() && (std::find(loadedFiles.begin(), loadedFiles.end(), image_name) != loadedFiles.end());
		/// Load if it is a new one
		mitk::Image* img = dynamic_cast<mitk::Image*>(baseData.GetPointer());
		if (!isInDS && img)
		{
			this->AddImage(image_name, img);
		}
	}

	/// Initialize views as axial, sagittal, coronar (from top-left to bottom)
	auto geo = this->m_DataStorage->ComputeBoundingGeometry3D(this->m_DataStorage->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews(geo);

	return 0;
}