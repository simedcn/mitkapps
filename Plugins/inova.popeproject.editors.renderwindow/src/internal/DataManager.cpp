#include "DataManager.h"
#include "PatientSelector.h"
#include "SpacingSelector.h"
#include <PopeElements.h>
#include "PopePreferencePage.h"
#include "inova_popeproject_editors_renderwindow_Activator.h"

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <mitkIOUtil.h>
#include <mitkRenderingManager.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProgressBar.h>
#include <mitkImagePixelReadAccessor.h>

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

	/// CTK slots.
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
	string filepath = filenameAndDirectory.toLocal8Bit().constData();
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

	//bool is_first = true;
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
				//if (is_first)
				//{
					//datanode->SetSelected(true);
				//
				//	is_first = false;
				//}
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
		string filepath = correct_file.toLocal8Bit().constData();

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
		if (index < 0 || index >= (int) patientIDs.size())
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
		string filepath = file.toLocal8Bit().constData();
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
void DataManager::On_ToolsPlugin_AllNodesRemoved(const ctkEvent&)
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

bool DataManager::IsTimeGeometryOK(mitk::Image::Pointer image)
{
	static const bool skip_first_and_last_elements = true;
	static const float threshold_step = 3.0f;
	static const float threshold_duration = 3.0f;

	auto tg = image->GetTimeGeometry();
	//uint num_time_frames = image->GetTimeSteps();
	uint num_time_frames = tg->CountTimeSteps();
	if (num_time_frames > 1)
	{
		mitk::TimePointType last_time = tg->GetMinimumTimePoint(0);
		vector<uint> step(num_time_frames);
		vector<uint> duration(num_time_frames);
		for (uint i = 0; i < num_time_frames; i++)
		{
			mitk::TimePointType t1 = tg->GetMinimumTimePoint(i);
			mitk::TimePointType t2 = tg->GetMaximumTimePoint(i);
			step[i] = t1 - last_time;
			duration[i] = t2 - t1;
			last_time = t2;
		}
		// Calculate std.dev.
		float mean_step = Elements::get_mean(step);
		float mean_duration = Elements::get_mean(duration);
		float std_dev_step = Elements::get_std_dev(step, &mean_step);
		float std_dev_duration = Elements::get_std_dev(duration, &mean_duration);
		bool are_steps_OK = true;
		vector<bool> wrong_step(num_time_frames);
		bool are_durations_OK = true;
		vector<bool> wrong_duration(num_time_frames);
		bool are_middle_elements_OK = true; // For the case when we skip first and last time frames.
		std::stringstream ss;
		ss << "The image contains " << num_time_frames << " frames (1.." << num_time_frames << ").\n\nPlease check the following time frames, which may be inconsistent: [";
		for (uint i = 0; i < num_time_frames; i++)
		{
			bool are_prev_elements_ok = (are_steps_OK && are_durations_OK);
			if (step[i] < mean_step - threshold_step * std_dev_step ||
				step[i] > mean_step + threshold_step * std_dev_step)
			{
				are_steps_OK = false;
				wrong_step[i] = true;
			}
			else
			{
				wrong_step[i] = false;
			}
			if (duration[i] < mean_duration - threshold_duration * std_dev_duration ||
				duration[i] > mean_duration + threshold_duration * std_dev_duration)
			{
				are_durations_OK = false;
				wrong_duration[i] = true;
			}
			else
			{
				wrong_duration[i] = false;
			}
			if (wrong_step[i] || wrong_duration[i])
			{
				if (are_prev_elements_ok)
					ss << i + 1;
				else
					ss << ", " << i + 1;
				if (i != 0 && i != num_time_frames - 1)
					are_middle_elements_OK = false;
			}
		}
		if (!are_middle_elements_OK || (!skip_first_and_last_elements && (!are_steps_OK || !are_durations_OK)))
		{
			ss << "].\n\nWould you like to load this data set? As an option, you can download this data set and further deselect inconsistent frames/timepoints.";
			QMessageBox msgBox;
			msgBox.setText("The data being loaded may contain inconsistent frames.");
			msgBox.setInformativeText(QString::fromStdString(ss.str()));
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::Yes);
			int ret = msgBox.exec();
			if (ret == QMessageBox::No)
				return false;
		}
	}
	return true;
}
mitk::Point3D GetMean(const vector<mitk::Point3D> pts)
{
	mitk::Point3D mean;
	for (const auto pt : pts)
	{
		for (uint i = 0; i < 3; i++)
		{
			mean[i] += pt[i];
		}
	}
	for (uint i = 0; i < 3; i++)
	{
		mean[i] /= pts.size();
	}
	return mean;
}
mitk::Point3D GetStd(const vector<mitk::Point3D> pts, const mitk::Point3D mean)
{
	mitk::Point3D stdev;
	for (uint i = 0; i < 3; i++)
	{
		std::vector<float> values(pts.size());
		for (uint j = 0; j < pts.size(); j++)
		{
			values[j] = pts[i][j];
		}
		std::vector<float> diff(values.size());
		std::transform(values.begin(), values.end(), diff.begin(), [mean, i](float x) { return x - (float)(mean[i]); });
		float sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0f);
		stdev[i] = std::sqrt(sq_sum / values.size());
	}
	return stdev;
}
template<typename VoxelDataType>
float GetMean(const std::vector<VoxelDataType>& v)
{
	ulong sum = std::accumulate(v.begin(), v.end(), 0.0f);
	float mean = float(sum) / v.size();
	return mean;
}
template<typename VoxelDataType>
float GetStd(const std::vector<VoxelDataType>& v, const float& mean)
{
	std::vector<float> diff(v.size());
	std::transform(v.begin(), v.end(), diff.begin(), [mean](float x) { return x - mean; });
	float sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0f);
	float stdev = std::sqrt(sq_sum / v.size());
	return stdev;
}
bool DataManager::IsCentralRegionOK(mitk::Image::Pointer image)
{
	static const float threshold_mean = 3.0f;

	try
	{
		/// Get image dimension.
		uint num_dim = image->GetDimension();
		if (num_dim < 3)
			return true;
		uint* dimensions = image->GetDimensions();
		/// Get the center point.
		using VoxelDataType = unsigned short;
		mitk::ImagePixelReadAccessor<VoxelDataType, 3U> readAccess(image);
		vector<vector<uint>> center_coords(3);
		for (uint n = 0; n < 3; n++)
		{
			assert(dimensions[n] > 0);
			if (dimensions[n] == 0)
				return true;
			if (dimensions[n] % 2 == 0)
			{
				center_coords[n].push_back(dimensions[n] / 2);
			}
			else
			{
				if (dimensions[n] == 1)
					center_coords[n].push_back(dimensions[n] / 2);
				else
				{
					center_coords[n].push_back(dimensions[n] / 2);
					center_coords[n].push_back(dimensions[n] / 2 + 1);
				}
			}
		}
		vector<VoxelDataType> center_pts;
		for (uint n = 0; n < 3; n++)
		{
			for (uint i = 0; i < center_coords[0].size(); i++)
			{
				for (uint j = 0; j < center_coords[1].size(); j++)
				{
					for (uint k = 0; k < center_coords[2].size(); k++)
					{
						//uint pt[3] = { center_coords[0][i], center_coords[1][j], center_coords[2][k] };
						//mitk::Point3D point(pt[3]);
						itk::Index<3U> point;
						point[0] = center_coords[0][i];
						point[1] = center_coords[1][j];
						point[2] = center_coords[2][k];
						VoxelDataType world_pt = readAccess.GetPixelByIndex(point);
						center_pts.push_back(world_pt);
					}
				}
			}
		}
		float center_value = GetMean(center_pts);
		/// Get the area around the center.
		vector<vector<uint>> around_center_coords(3);
		const uint step = 3;
		for (uint n = 0; n < 3; n++)
		{
			uint i1 = max(0U, dimensions[n] / 2 - step);
			uint i2 = min(dimensions[n] - 1, dimensions[n] / 2 + step);
			for (uint i = i1; i <= i2; i++)
			{
				if (i != dimensions[n] && (dimensions[n] % 2 == 0 || i != dimensions[n] / 2 + 1))
				{
					around_center_coords[n].push_back(i);
				}
			}
		}
		vector<VoxelDataType> around_center_pts;
		for (uint n = 0; n < 3; n++)
		{
			for (uint i = 0; i < around_center_coords[0].size(); i++)
			{
				for (uint j = 0; j < around_center_coords[1].size(); j++)
				{
					for (uint k = 0; k < around_center_coords[2].size(); k++)
					{
						//uint pt[3] = { around_center_coords[0][i], around_center_coords[1][j], around_center_coords[2][k] };
						//mitk::Point3D point(pt[3]);
						itk::Index<3U> point;
						point[0] = around_center_coords[0][i];
						point[1] = around_center_coords[1][j];
						point[2] = around_center_coords[2][k];
						VoxelDataType world_pt = readAccess.GetPixelByIndex(point);
						around_center_pts.push_back(world_pt);
					}
				}
			}
		}
		float around_center_value = GetMean(around_center_pts);
		float around_center_std = GetStd(around_center_pts, around_center_value);

		/// Compare values Center vs Around Center
		if (center_value < around_center_value - threshold_mean * around_center_std
			|| center_value > around_center_value + threshold_mean * around_center_std)
		{
			std::stringstream ss;
			ss << "The image may contain the central point artefact resulted from a constant direct current offset in the receiver electronics. "
			<< "\n\nWould you like to load this data set?";
			QMessageBox msgBox;
			msgBox.setText("Possible central point artefact.");
			msgBox.setInformativeText(QString::fromStdString(ss.str()));
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::Yes);
			int ret = msgBox.exec();
			if (ret == QMessageBox::No)
				return false;
		}
	}
	catch (...)
	{}
	return true;
}
void DataManager::CheckImageSpacing(mitk::Image::Pointer image, const string& image_name)
{
	CheckVoxelSpacing check_spacing = (CheckVoxelSpacing) DataManager::getPreferencesNode()->GetInt("check spacing", CheckVoxelSpacing::CheckOnlyIf01);
	if (check_spacing == CheckVoxelSpacing::UseDefaultValueWithoutChecking)
		return;
	/// Get a spacing from mitk::Image.
	auto spacing = image->GetSlicedGeometry()->GetSpacing();
	if (spacing.Size() < 3)
		return;
	/// Get DICOM keys/tags: PixelSpacing, SliceLocation, SpacingBetweenSlices.
	//positions.push_back(Elements::get_property("dicom.patient.ImagePosition", "DICOM.0020.0032", baseData));
	//string key_PixelSpacing;
	//string key_SliceLocation;
	//vector<string> props = image->GetPropertyKeys();
	//vector<string> props_lowercase(props.size());
	//for (uint i = 0; i < props.size(); i++)
	//{
	//	props_lowercase[i] = props[i];
	//	Elements::to_lower(props_lowercase[i]);
	//}
	//for (const string& prop : props_lowercase)
	//{
	//	if (prop.find("DICOM.0028.0030") >= 0) // Pixel Spacing
	//		key_PixelSpacing = prop;
	//	else if (prop.find("dicom.image.0020.1041") >= 0) // Pixel Spacing
	//		key_SliceLocation = prop;
	//}
	string tag_PixelSpacing; // DICOM.0028.0030
	string tag_SliceLocation; // dicom.image.0020.1041
	string tag_SpacingBetweenSlices; // DICOM.0018.0088
	auto props = image->GetPropertyList();
	auto p = props->GetProperty("DICOM.0028.0030");
	if (p)
		tag_PixelSpacing = p->GetValueAsString(); // tag_PixelSpacing = "1.79687500000000\\1.79687500000000"
	p = props->GetProperty("DICOM.0018.0088");
	if (p)
		tag_SpacingBetweenSlices = p->GetValueAsString(); // tag_SpacingBetweenSlices = "5.5"
	p = props->GetProperty("dicom.image.0020.1041");
	if (p)
		tag_SliceLocation = p->GetValueAsString(); // tag_SliceLocation = "[0 -> -57.190132579244, 1 -> -51.690132720319, 2 -> -46.19013286056, 3 -> -40.690132602608, 4 -> -35.190134638984, 5 -> -29.690134779225, 6 -> -24.190134719953, 7 -> -18.690133913795, 8 -> -13.1901338...
	/// Process PixelSpacing.
	float value_PixelSpacing = -1.0f;
	if (!tag_PixelSpacing.empty())
	{
		try
		{
			vector<float> values_PixelSpacing;
			bool is_ok = Elements::split_property(tag_PixelSpacing, &values_PixelSpacing);
			if (is_ok && values_PixelSpacing.size() == 3 && abs(values_PixelSpacing[2]) > 0.01f)
				value_PixelSpacing = values_PixelSpacing[2];
		}
		catch (...)
		{
			value_PixelSpacing = -1.0f;
		}
	}
	/// Process SpacingBetweenSlices.
	float value_SpacingBetweenSlices = -1.0f;
	if (!tag_SpacingBetweenSlices.empty())
	{
		try
		{
			value_SpacingBetweenSlices = stof(tag_SpacingBetweenSlices);
		}
		catch (...)
		{
			value_SpacingBetweenSlices = -1.0f;
		}
	}
	/// Calculate a spacing from SliceLocation.
	float value_SliceLocation = -1.0f;
	try
	{
		QStringList locations;
		bool is_ok = Elements::split_properties(tag_SliceLocation, &locations);
		//for (auto pr : locations)
		//	MITK_INFO << pr.toStdString();
		if (!is_ok)
			throw("Split exception");
		bool is_first = true;
		float prev_val;
		vector<float> diff;
		for (const auto& loc : locations)
		{
			bool is_ok = false;
			float val = loc.toFloat(&is_ok);
			if (!is_ok)
				throw("Float exception");
			if (is_first)
				is_first = false;
			else
				diff.push_back(abs(val - prev_val));
			prev_val = val;
		}
		if (diff.size() > 0)
		{
			float mean_spacing = Elements::get_mean(diff);
			float std_dev_spacing = Elements::get_std_dev(diff, &mean_spacing);
			const float& threshold_stddev = 0.05f;
			assert(mean_spacing >= -0.0001f);
			if (mean_spacing > 0.01f && std_dev_spacing < threshold_stddev * mean_spacing)
				value_SliceLocation = mean_spacing;
		}
	}
	catch (...)
	{
		value_SliceLocation = -1.0f;
	}

	/// Compare the spacings.
	float def_spacing = (float)spacing[2];
	if (check_spacing == CheckVoxelSpacing::CheckAlways
		|| (check_spacing == CheckVoxelSpacing::CheckOnlyIf01 && (abs(def_spacing - 1.0f) < 0.0001f || def_spacing < 0.0001f)))
	{// spacing may be wrong
		vector<shared_ptr<SpacingDescriptor>> spacings;
		spacings.push_back(make_shared<SpacingDescriptor>("Image Position (Patient)", "0020,0032", def_spacing));
		if (value_PixelSpacing > 0)
			spacings.push_back(make_shared<SpacingDescriptor>("Pixel Spacing", "0028,0030", value_PixelSpacing));
		if (value_SpacingBetweenSlices > 0)
			spacings.push_back(make_shared<SpacingDescriptor>("Spacing Between Slices", "0018,0088", value_SpacingBetweenSlices));
		if (value_SliceLocation > 0)
			spacings.push_back(make_shared<SpacingDescriptor>("Slice Location", "0020,1041", value_SliceLocation));
		if (spacings.size() > 1)
		{
			bool are_consistent = (def_spacing >= 0.0001f);
			if (are_consistent)
			{
				const float& threshold_rel = 0.01f;
				const float& threshold_abs = 0.3f;
				for (size_t i = 1; i < spacings.size(); i++)
				{
					float diff = abs(spacings[i]->value - def_spacing);
					if (diff > threshold_rel * def_spacing || diff > threshold_abs)
					{
						are_consistent = false;
						break;
					}
				}
			}
			if (!are_consistent)
			{
				SpacingSelector spacingSelector(nullptr);
				spacingSelector.SetImageDescription(QString::fromStdString(image_name));
				spacingSelector.SetData(spacings);
				int retval = spacingSelector.exec();
				if (retval == QDialog::Accepted)
				{
					float new_value = -1.0f;
					int index = spacingSelector.SelectedSpacingIndex(&new_value);
					if (index > 0 && index < (int) spacings.size()) // (index == 0) --> default value
					{
						assert(new_value == spacings[index]->value);
						new_value = spacings[index]->value;
					}
					//else: use new_value from SelectedSpacingIndex()
					if (index != 0 && new_value > 0) // (index == 0) --> default value
					{
						MITK_INFO << "Slice Spacing was changed from " << std::fixed << std::setprecision(2) << spacing[2]
							<< " [Default value] to " << new_value << ".";
						spacing[2] = new_value;
						image->SetSpacing(spacing);
					}
				}
			}
		}
	}
}

mitk::DataNode::Pointer DataManager::AddImage(const string& name, mitk::Image::Pointer image)
{
	//bool isInDS = false;/// Check if the datanode is already loaded.
	auto allNodes = this->m_DataStorage->GetAll();
	auto iterator = allNodes->Begin();
	vector<string> positions;
	while (iterator != allNodes->End())
	{
		mitk::DataNode::Pointer datanode = iterator->Value();
		auto baseData = datanode->GetData();
		++iterator;
		mitk::Image* image_i = dynamic_cast<mitk::Image*>(baseData);
		if (!image_i)
			continue;
		if (datanode->GetName().compare(name) == 0)
		{
			//isInDS = true;
			return datanode;
		}
	}
	/// If not loaded:
	/// Check the time geometry.
	bool is_ok = IsTimeGeometryOK(image);
	if (!is_ok)
		return nullptr;

	/// Check the central region.
	is_ok = IsCentralRegionOK(image);
	if (!is_ok)
		return nullptr;

	/// Check the voxel spacing.
	CheckImageSpacing(image, name);

	/// Create and add a new node.
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
			mitk::IOUtil::Save(image, (string)path.toLocal8Bit().constData());
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
			mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>((string)segFile.toLocal8Bit().constData());
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
