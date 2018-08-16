#include "DataManager.h"
#include "PatientSelector.h"
#include <PopeElements.h>
#include "inova_popeproject_editors_renderwindow_Activator.h"

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <mitkIOUtil.h>
#include <mitkRenderingManager.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProgressBar.h>

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

#include <memory>
#include <map>
#include <vector>
#include <set>
#include <utility>


//QSettings DataManager::m_Settings("savedFiles.ini", QSettings::IniFormat);
berry::IPreferences::Pointer DataManager::m_PreferencesNode;

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

DataManager::DataManager(mitk::DataStorage *datastorage, QObject *parent)
	: QObject(parent), m_DataStorage(datastorage)
{
	//bool load_nrrd = false;
	//m_Settings.setValue("AppSettings/LoadRNND", load_nrrd); //??

	/// CTK signals.
	auto pluginContext = inova_popeproject_editors_renderwindow_Activator::GetPluginContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "data/OPENDICOMDATASET";
		eventAdmin->subscribeSlot(this, SLOT(on_Action_OpenDICOMdataset_clicked(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "data/OPENFOLDER";
		eventAdmin->subscribeSlot(this, SLOT(on_Action_OpenFolder_clicked(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "data/OPENDICOMSERIES";
		eventAdmin->subscribeSlot(this, SLOT(on_Action_OpenDICOMSeries_clicked(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "data/ALLNODESREMOVED";
		eventAdmin->subscribeSlot(this, SLOT(On_ToolsPlugin_AllNodesRemoved(ctkEvent)), propsForSlot);
		//propsForSlot[ctkEventConstants::EVENT_TOPIC] = "data/ADDMITKIMAGE";
		//eventAdmin->subscribeSlot(this, SLOT(on_Action_LoadImage(ctkEvent)), propsForSlot);
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

int DataManager::on_LoadImageSet(const QString &filenameAndDirectory)
{
	if (filenameAndDirectory.size() == 0)
		return -9;

	/// Check the file if it is readable.
	string filepath = filenameAndDirectory.toStdString();
	QString patientId = Elements::get_patientId_or_patientName(filepath);
	if (patientId.isEmpty())
		return -1;

	/// Check if the image is from the same patient.
	bool toAsk = (!m_PatientId.isEmpty() && patientId != m_PatientId);
	if (toAsk)
	{
		int retval = AskAboutNewPatient();
		if (retval != 0)
			return retval;
	}

	/// Update PatientId, CurrentFolder, and WorkDirectory.
	m_PatientId = patientId;
	QFileInfo fi(filenameAndDirectory);
	SetWorkDirectory(fi.dir().absolutePath());

	/// Load data.
	vector<string> loadedFiles = this->LoadDataOfCurrentPatient();
	auto loaded = mitk::IOUtil::Load(filepath);

	/// Set progressBar.
	auto progressbar = mitk::ProgressBar::GetInstance();
	if (loaded.size() > 0)
	{
		progressbar->Reset();
		progressbar->AddStepsToDo(loaded.size());
	}

	bool is_first = true;
	for (const auto baseData : loaded)
	{
		//mitk::DataNode::Pointer dn = mitk::DataNode::New();
		/// Get patient ID
		string curPatID = Elements::get_patientId_or_patientName(baseData);
		if (patientId == QString::fromStdString(curPatID))
		{
			/// Generate an image name.
			string image_name = Elements::get_imageName(baseData);
			//MITK_INFO << image_name;
			/// Detect if the datanode is already saved.
			bool isInDS = !loadedFiles.empty() && (std::find(loadedFiles.begin(), loadedFiles.end(), image_name) != loadedFiles.end());
			/// Load if it is a new one.
			mitk::Image* img = dynamic_cast<mitk::Image*>(baseData.GetPointer());
			if (!isInDS && img)
			{
				auto datanode = this->AddImage(image_name, img);
				/// Select the first node loaded.
				if (is_first)
				{
					//datanode->SetSelected(true);
					

					is_first = false;
				}
			}
		}
		progressbar->Progress();
	}
	/// Initialize views as axial, sagittal, coronar (from top-left to bottom).
	auto geo = this->m_DataStorage->ComputeBoundingGeometry3D(this->m_DataStorage->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews(geo);
	//this->InitializeView();

	progressbar->Reset();
	return 0;
}
int DataManager::on_LoadImageFolder(const QString& directory)
{
	if (directory.size() == 0)
		return -19;

	/// Get all subfolders if needed.
	list<QDir> dirs;
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

	/// Set progressBar.
	auto progressbar = mitk::ProgressBar::GetInstance();
	progressbar->Reset();
	progressbar->AddStepsToDo(dirs.size() + 25);
	
	/// Go through all the dirs.
	vector<mitk::BaseData::Pointer> all_loaded;
	for (auto& dir : dirs)
	{
		/// Get all DCM files from the directory.
		dir.setFilter(QDir::Files);
		dir.setNameFilters(QStringList("*.dcm"));
		auto files = dir.entryList();
		//string path = dir.absolutePath().toStdString();
		//int num = files.size();

		/// Find the first correct (readable) file and delete wrong files from the list.
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
		progressbar->Progress();

		if (correct_file.isEmpty())
			continue;
		string filepath = correct_file.toStdString();

		/// Read data.
		auto loaded = mitk::IOUtil::Load(filepath);
		if (loaded.size() == 0)
		{
			progressbar->Reset();
			return -11;
		}

		all_loaded.insert(all_loaded.end(), loaded.begin(), loaded.end());
	}
	if (all_loaded.size() == 0)
	{
		progressbar->Reset();
		return -12;
	}

	/// Group the data by patient id.
	map<string, list<mitk::BaseData::Pointer>> groups;
	for (const auto baseData : all_loaded)
	{
		/// Get patient ID.
		string patientID = Elements::get_patientId_or_patientName(baseData);
		groups[patientID].push_back(baseData);
	}

	/// Get all patient IDs.
	vector<string> patientIDs(groups.size());
	vector<shared_ptr<PatientDescription>> patientDescriptors(groups.size());
	int i = 0;
	for (const auto patient : groups)
	{
		const string& id = patient.first;
		patientIDs[i] = id;
		QString str_id = QString::fromStdString(id);
		const auto& baseDataList = patient.second;
		string name = Elements::find_patientName(baseDataList);
		string patientName = Elements::recognize_name(name);
		QString str_name = QString::fromStdString(patientName);
		auto images = Elements::get_imageNames(baseDataList);
		patientDescriptors[i] = make_shared<PatientDescription>(str_id, str_name, images);
		i++;
	}

	/// Combine images if one patient has name and one is unnamed.
	if (patientDescriptors.size() == 2 && (patientIDs[0].empty() != patientIDs[1].empty()))
	{
		assert(patientDescriptors.size() == 2 && groups.size() == 2);
		string patientID = (patientIDs[0].empty()) ? patientIDs[1] : patientIDs[0];
		auto& list_images = groups[patientID];
		list_images.splice(list_images.end(), groups[""]);
		groups.erase("");
		assert(groups.size() == 1);
	}

	/// Select the patient.
	int index = 0;
	if (groups.size() > 1)
	{
		PatientSelector patientSelector(nullptr);
		patientSelector.SetPatientData(patientDescriptors);
		patientSelector.SetFolder(directory);
		int retval = patientSelector.exec();
		if (retval != QDialog::Accepted)
		{
			progressbar->Reset();
			return -13;
		}
		index = patientSelector.SelectedPatientIDIndex();
		if (index < 0 || index >= patientIDs.size())
		{
			progressbar->Reset();
			return -14;
		}
	}
	string selected_id = patientIDs[index];

	/// Check if the image is from the same patient, which is already loaded in DataManager.
	bool toAsk = !m_PatientId.isEmpty() && (selected_id != m_PatientId.toStdString());
	if (toAsk)
	{
		int retval = AskAboutNewPatient();
		if (retval != 0)
		{
			progressbar->Reset();
			return retval;
		}
	}

	/// Update PatientId, CurrentFolder, and WorkDirectory.
	m_PatientId = QString::fromStdString(selected_id);
	SetWorkDirectory(directory);

	/// Load data.
	progressbar->Reset();
	progressbar->AddStepsToDo(groups[selected_id].size());
	vector<string> loadedFiles = this->LoadDataOfCurrentPatient();
	for (const auto baseData : groups[selected_id])
	{
		/// Generate an image name.
		string image_name = Elements::get_imageName(baseData);
		//MITK_INFO << image_name;
		/// Detect if the datanode is already saved.
		bool isInDS = !loadedFiles.empty() && (std::find(loadedFiles.begin(), loadedFiles.end(), image_name) != loadedFiles.end());
		/// Load if it is a new one.
		mitk::Image* img = dynamic_cast<mitk::Image*>(baseData.GetPointer());
		if (!isInDS && img)
		{
			auto datanode = this->AddImage(image_name, img);
		}
		progressbar->Progress();
	}

	/// Initialize views as axial, sagittal, coronar (from top-left to bottom).
	auto geo = this->m_DataStorage->ComputeBoundingGeometry3D(this->m_DataStorage->GetAll());
	mitk::RenderingManager::GetInstance()->InitializeViews(geo);

	progressbar->Reset();
	return 0;
}
int DataManager::on_LoadImageSeries(const QStringList& files, const QStringList& series)
{
	int error_code = 0;
	for (const auto& file : files)
	{
		if (error_code != 0)
		{
			QString text = "Do you want to continue reading the files retrieved and saved on the disk?";
			auto msg_res = QMessageBox::question(nullptr, tr("Add to Data Manager"), text, QMessageBox::Yes | QMessageBox::No);
			if (msg_res != QMessageBox::Yes)
				return error_code;
		}
		string filepath = file.toStdString();
		QString seriesInstanceUID = Elements::get_seriesInstanceUID(filepath);
		if (seriesInstanceUID.isEmpty())
		{//??
			//continue;
			MITK_INFO << "Unable to read seriesInstanceUID from the file \"" << filepath << "\". Solution: load all files from the folder.";
			QString dir_path = QFileInfo(file).absolutePath();
			error_code = on_LoadImageFolder(dir_path);
			return error_code;
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
		error_code = on_LoadImageSet(file);
	}
	return 0;
}
/*int DataManager::on_LoadImage(const string& name, mitk::Image::Pointer image)
{
	/// Assuming it's the same patient.
	auto datanode = AddImage(name, image);
	if (datanode == nullptr)
		return -1;
	return 0;
}*/
void DataManager::on_Action_OpenDICOMdataset_clicked(const ctkEvent& event)
{
	QString imagePath = event.getProperty("imagePath").toString();
	on_LoadImageSet(imagePath);
}
void DataManager::on_Action_OpenFolder_clicked(const ctkEvent& event)
{
	QString imagePath = event.getProperty("imagePath").toString();
	on_LoadImageFolder(imagePath);
}
void DataManager::on_Action_OpenDICOMSeries_clicked(const ctkEvent& event)
{
	QStringList files = event.getProperty("files").toStringList();
	QStringList series = event.getProperty("series").toStringList();
	on_LoadImageSeries(files, series);
}
void DataManager::On_ToolsPlugin_AllNodesRemoved(const ctkEvent& event)
{
	m_PatientId = "";
}
/*void DataManager::on_Action_LoadImage(const ctkEvent& event)
{
	string name = event.getProperty("name").toString().toStdString();
	mitk::Image::Pointer image;// = event.getProperty("image").value<mitk::Image::Pointer>();
	mitk::Image::Pointer dn = event.getProperty("image").value<ImageEvent>().image;
	on_LoadImage(name, image);
}*/

mitk::DataNode::Pointer DataManager::AddImage(const string& name, mitk::Image::Pointer image)
{
	bool isInDS = false;//is the datanode already loaded?
	auto allNodes = this->m_DataStorage->GetAll();
	auto iterator = allNodes->Begin();
	while (iterator != allNodes->End())
	{
		mitk::DataNode::Pointer datanode = iterator->Value();
		++iterator;
		mitk::Image* image_i = dynamic_cast<mitk::Image*>(datanode->GetData());
		if (!image_i)
			continue;
		if (datanode->GetName().compare(name) == 0)
		{
			isInDS = true;
			return datanode;
		}
	}
	//if !isInDS:
	mitk::DataNode::Pointer dn = mitk::DataNode::New();
	dn->SetName(name);
	dn->SetData(image);
	emit GonnaAddNewDataNode();
	this->m_DataStorage->Add(dn);
	emit NewDataNodeAdded();
	return dn;
}

void DataManager::SaveDataOfCurrentPatient()
{
	auto allNodes = this->m_DataStorage->GetAll();
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	QString node_name = "/saved.patients." + this->m_PatientId;
	auto node = prefService->GetSystemPreferences()->Node(node_name);
	for (auto it = allNodes->Begin(); it != allNodes->End(); ++it)
	{
		mitk::DataNode::Pointer datanode = it->Value();
		mitk::Image* image = dynamic_cast<mitk::Image*>(datanode->GetData());
		if (image)
		{
			QString name = QString::fromStdString(datanode->GetName());
			QString filename = name;
			filename.append(".nrrd");
			QString currentPath = GetWorkDirectory();
			QString path = currentPath + "/" + filename;
			mitk::IOUtil::Save(image, path.toStdString());
			//m_Settings.beginGroup(this->m_PatientId);
			//m_Settings.setValue(key, path);
			//m_Settings.endGroup();
			node->Put(name, path);
		}
	}
}
std::vector<std::string> DataManager::LoadDataOfCurrentPatient()
{
	std::vector<std::string> loadedFiles;
	QString currentPath = GetWorkDirectory();
	if (currentPath.isEmpty() || this->m_PatientId.isEmpty())
		return loadedFiles;
	bool load_nrrd = DataManager::getPreferencesNode()->GetBool("load rnnd", false);
	if (!load_nrrd)
		return loadedFiles;
	//m_Settings.beginGroup(this->m_PatientId);
	//auto keys = m_Settings.childKeys();
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	QString node_name = "/saved.patients." + this->m_PatientId;
	auto node = prefService->GetSystemPreferences()->Node(node_name);
	auto keys = node->ChildrenNames();
	for (const auto& key : keys)
	{ //dicom.series.SeriesInstanceUID
		//QString segFile = m_Settings.value(key).toString();
		QString segFile = node->Get(key, "");
		if (!segFile.isEmpty())
		{
			mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(segFile.toStdString());
			this->AddImage(key.toStdString(), image);
			loadedFiles.push_back(key.toStdString());
		}
	}
	//m_Settings.endGroup(); //patientId
	return loadedFiles;
}

berry::IPreferences::Pointer DataManager::getPreferencesNode()
{
	if (m_PreferencesNode.IsNotNull())
		return m_PreferencesNode;
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	m_PreferencesNode = prefService->GetSystemPreferences()->Node("/inova.popeproject.editors.renderwindow");
	return m_PreferencesNode;
}
QString DataManager::GetWorkDirectory()
{
	//return m_Settings.value("AppSettings/WorkDirectory").toString();
	QString def_path = QCoreApplication::applicationDirPath();
	QString path = DataManager::getPreferencesNode()->Get("data folder", def_path);
	return path;
}
void DataManager::SetWorkDirectory(const QString& dir_path)
{
	//m_Settings.setValue("AppSettings/WorkDirectory", dir_path);
	getPreferencesNode()->Put("data folder", dir_path);
}