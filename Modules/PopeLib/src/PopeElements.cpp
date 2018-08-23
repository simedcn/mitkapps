#include <PopeElements.h>

#include <itkGDCMImageIO.h>

#include <sstream>
#include <algorithm>
#include <iterator>
#include <assert.h>

using namespace std;


Elements::Elements()
{}

void Elements::delete_spaces(string& str)
{
	str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
}
void Elements::to_upper(string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}
void Elements::to_lower(string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}
string Elements::trim_string(const string& str, const string& whitespace)
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}
wstring Elements::trim_string(const wstring& str, const wstring& whitespace)
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == string::npos)
		return L""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

string Elements::get_property(const char* prop_a, const char* prop_b, const mitk::BaseData* data, bool show_warning, const string& def_value)
{
	if (data == nullptr)
		return def_value;

	auto a = data->GetProperty(prop_a);
	if (a)
		return a->GetValueAsString();

	auto b = data->GetProperty(prop_b);
	if (b)
		return b->GetValueAsString();

	if (show_warning)
		MITK_WARN << "Failed to read the DICOM tag " << prop_a << " (" << prop_b << ")";
	return def_value;
}
string Elements::get_property(const char* prop_a, const char* prop_b, const mitk::DataNode* dataNode, bool show_warning, const string& def_value)
{
	if (dataNode == nullptr)
		return def_value;

	const auto data = dataNode->GetData();
	return get_property(prop_a, prop_b, data, show_warning, def_value);
}
string Elements::get_patientId(mitk::BaseData* baseData, bool show_warning, const string& def_value)
{
	return get_property("dicom.patient.PatientID", "DICOM.0010.0020", baseData, show_warning, def_value);
}
string Elements::get_patientId(mitk::BaseData::Pointer baseData, bool show_warning, const string& def_value)
{
	if (baseData == nullptr)
		return def_value;

	return get_patientId(baseData.GetPointer(), show_warning, def_value);
}
string Elements::get_patientId(mitk::DataNode* dataNode, bool show_warning, const string& def_value)
{
	if (dataNode == nullptr)
		return def_value;

	const auto data = dataNode->GetData();
	return get_patientId(data, show_warning, def_value);
}
string Elements::get_patientId_or_patientName(mitk::BaseData* baseData, bool show_warning, const string& def_value)
{
	string id = get_patientId(baseData, show_warning, "");
	if (!id.empty())
		return id;

	id = get_patientName(baseData, show_warning, "");
	if (!id.empty())
		return id;

	return def_value;
}
string Elements::get_patientId_or_patientName(mitk::BaseData::Pointer baseData, bool show_warning, const string& def_value)
{
	if (baseData == nullptr)
		return def_value;

	return get_patientId_or_patientName(baseData.GetPointer(), show_warning, def_value);
}
string Elements::get_patientId_or_patientName(mitk::DataNode* dataNode, bool show_warning, const string& def_value)
{
	if (dataNode == nullptr)
		return def_value;

	const auto data = dataNode->GetData();
	return get_patientId_or_patientName(data, show_warning, def_value);
}
QString Elements::get_patientId_or_patientName(const string& filename, bool show_warning, const QString& def_value)
{
	itk::GDCMImageIO::Pointer reader = itk::GDCMImageIO::New();
	if (!reader->CanReadFile(filename.c_str()))
		return def_value;
	reader->SetFileName(filename);
	reader->ReadImageInformation();
	char id[512];
	reader->GetPatientID(id);
	QString patientId = id;
	if (!patientId.isEmpty())
		return patientId;
	reader->GetPatientName(id);
	patientId = id;
	if (!patientId.isEmpty())
		return patientId;
	if (show_warning)
		MITK_WARN << "No patientID in " << filename;
	return def_value;
}
QString Elements::get_patientId_or_patientName(const QString& filename, bool show_warning, const QString& def_value)
{
	string name = filename.toStdString();
	return get_patientId_or_patientName(name, show_warning, def_value);
}
QString Elements::get_seriesInstanceUID(const string& filename, const QString& def_value)
{
	try
	{
		itk::GDCMImageIO::Pointer reader = itk::GDCMImageIO::New();
		if (!reader->CanReadFile(filename.c_str()))
			return def_value;
		reader->SetFileName(filename);
		reader->ReadImageInformation();
		QString seriesInstanceUID = reader->GetSeriesInstanceUID();
		return seriesInstanceUID;
	}
	catch (...)
	{
		return def_value;
	}
}
string Elements::get_patientName(mitk::BaseData* baseData, bool show_warning, const string& def_value)
{
	string name = get_property("dicom.patient.PatientsName", "DICOM.0010.0010", baseData, show_warning, def_value);
	if (name.empty())
	{
		name = get_property("dicom.patient_name", "DICOM.patient_name", baseData, show_warning, def_value);
	}
	return name;
}
string Elements::get_patientName(mitk::BaseData::Pointer baseData, bool show_warning, const string& def_value)
{
	if (baseData == nullptr)
		return def_value;

	return get_patientName(baseData.GetPointer(), show_warning, def_value);
}
string Elements::get_patientName(mitk::DataNode* dataNode, bool show_warning, const string& def_value)
{
	if (dataNode == nullptr)
		return def_value;

	const auto data = dataNode->GetData();
	return get_patientName(data, show_warning, def_value);
}
string Elements::find_patientName(const list<mitk::BaseData::Pointer>& baseDataList, bool show_warning, const string& def_value)
{
	if (baseDataList.size() == 0)
		return "";

	vector<string> props = { "DICOM.0010.0010", "dicom.patient.PatientsName", "dicom.patient_name", "DICOM.patient_name" };

	// Read all the data sets until the patient name is found
	for (const auto baseData : baseDataList)
	{
		if (baseData == nullptr || baseData.IsNull())
			continue;
		
		for (const auto& prop : props)
		{
			auto name = baseData->GetProperty(prop.c_str());
			if (name)
				return name->GetValueAsString();
		}
	}

	if (show_warning)
		MITK_WARN << "Failed to read the DICOM tag " << props[0] << " (" << props[1] << ")";
	return def_value;
}
string Elements::get_patientBirthdate(mitk::BaseData* baseData, bool show_warning, const string& def_value)
{
	return get_property("dicom.patient.PatientsBirthDate", "DICOM.0010.0030", baseData, show_warning, def_value);
}
string Elements::get_patientBirthdate(mitk::BaseData::Pointer baseData, bool show_warning, const string& def_value)
{
	if (baseData == nullptr)
		return def_value;

	return get_patientBirthdate(baseData.GetPointer(), show_warning, def_value);
}
string Elements::get_patientBirthdate(mitk::DataNode* dataNode, bool show_warning, const string& def_value)
{
	if (dataNode == nullptr)
		return def_value;

	const auto data = dataNode->GetData();
	return get_patientBirthdate(data, show_warning, def_value);
}
string Elements::get_patientGender(mitk::BaseData* baseData, bool show_warning, const string& def_value)
{
	return get_property("dicom.patient.PatientsSex", "DICOM.0010.0040", baseData, show_warning, def_value);
}
string Elements::get_patientGender(mitk::BaseData::Pointer baseData, bool show_warning, const string& def_value)
{
	if (baseData == nullptr)
		return def_value;

	return get_patientGender(baseData.GetPointer(), show_warning, def_value);
}
string Elements::get_patientGender(mitk::DataNode* dataNode, bool show_warning, const string& def_value)
{
	if (dataNode == nullptr)
		return def_value;

	const auto data = dataNode->GetData();
	return get_patientGender(data, show_warning, def_value);
}
string Elements::get_imageName(mitk::BaseData* baseData, bool show_warning, const string& def_value)
{
	if (baseData == nullptr)
		return def_value;

	stringstream imageName;
	//auto prop_SeriesInstanceUID = baseData->GetProperty("dicom.series.SeriesInstanceUID");
	string seriesInstanceUID = get_property("DICOM.0008.0018", "dicom.series.SeriesInstanceUID", baseData, show_warning);
	if (!seriesInstanceUID.empty())
		imageName << seriesInstanceUID; // prop_SeriesInstanceUID->GetValueAsString();
	else
	{
		seriesInstanceUID = get_property("DICOM.series_instance_uid", "dicom.series_instance_uid", baseData, show_warning);
		imageName << (seriesInstanceUID.empty() ? "image" : seriesInstanceUID);
	}
	//auto prop_Modality = baseData->GetProperty("dicom.series.Modality");
	string modality = get_property("DICOM.0008.0060", "dicom.series.Modality", baseData, show_warning);
	if (!modality.empty())
	{
		imageName << "_";
		imageName << modality; // prop_Modality->GetValueAsString();
	}
	return imageName.str();
}
string Elements::get_imageName(mitk::BaseData::Pointer baseData, bool show_warning, const string& def_value)
{
	if (baseData == nullptr)
		return def_value;

	return get_imageName(baseData.GetPointer(), show_warning, def_value);
}
string Elements::get_imageName(mitk::DataNode* dataNode, bool show_warning, const string& def_value)
{
	if (dataNode == nullptr)
		return def_value;

	const auto data = dataNode->GetData();
	return get_imageName(data, show_warning, def_value);
}
vector<QString> Elements::get_imageNames(const list<mitk::BaseData::Pointer>& baseDataList, bool show_warning, const string& def_value)
{
	// Use std::set to allow only unique image names
	set<QString> imageSet;
	for (const auto baseData : baseDataList)
	{
		string name = get_imageName(baseData, show_warning, def_value);
		imageSet.emplace(QString::fromStdString(name));
	}
	vector<QString> images;
	images.insert(images.end(), make_move_iterator(imageSet.begin()), make_move_iterator(imageSet.end()));
	return images;
}

string Elements::recognize_name(const string& name)
{
	string n = trim_string(name);
	replace(n.begin(), n.end(), '^', ' ');
	return n;
}
string Elements::recognize_gender(const string& gender)
{
	string gender_upper = trim_string(gender);
	to_upper(gender_upper);
	if (gender_upper == "F" || gender_upper == "W" || gender_upper == "FEMALE")
		return "Female";
	else if (gender_upper == "M" || gender_upper == "H" || gender_upper == "MALE")
		return "Male";
	return gender;
}
string Elements::recognize_birthday(const string& birthday)
{
	string b = trim_string(birthday);
	if (b.length() == 8)
	{
		try
		{
			int epoch = stoi(b.substr(0, 2));
			int year = stoi(b.substr(0, 4));
			int month = stoi(b.substr(4, 2));
			int day = stoi(b.substr(6, 2));
			if ((epoch == 18 || epoch == 19 || epoch == 20)
				&& month >= 1 && month <= 12
				&& day >= 1 && day <= 31)
			{
				stringstream ss;
				ss << std::setfill('0') << std::setw(2) << day << '.';
				ss << std::setfill('0') << std::setw(2) << month << '.';
				ss << year;
				return ss.str();
			}
		}
		catch (...)
		{
		}
	}
	return birthday;
}
string Elements::get_short_name(string name)
{
	const int max_length = 30;
	const int first_part = (max_length - 3 + 1) / 2;
	const int second_part = max_length - 3 - first_part;

	int len = name.length();
	if (len <= max_length)
		return name;

	stringstream short_name;
	short_name << name.substr(0, first_part) << "..." << name.substr(len - second_part - 1, second_part);
	return short_name.str();
}
QString Elements::get_short_name_for_image(const string& name)
{
	QString str_name = QString::fromStdString(name);
	QString short_name = str_name;
	const string str_center_replacement = "...";
	const int max_length = 28 + str_center_replacement.length();
	if (name.length() > max_length)
	{
		stringstream ss;
		const int half = (max_length - str_center_replacement.length()) / 2;
		ss << name.substr(0, half) << str_center_replacement << name.substr(name.length() - half, half);
		short_name = QString::fromStdString(ss.str());
	}
	return short_name;
}


bool Elements::recognize_property(QString* property, int* count)
{
	if (property == nullptr || count == nullptr)
		return false;
	QString& prop = *property;
	int& num = *count;

	int i1 = prop.indexOf(" -> ");
	if (i1 < 1 || i1 > 5 + 1)
	{
		return false;
	}
	bool ok = false;
	int number = prop.left(i1).trimmed().toInt(&ok);
	if (ok)
	{
		num = number;
		prop = prop.right(prop.length() - i1 - 4).trimmed();
		return true;
	}
	return false;
}
bool Elements::split_properties(const string& str_prop, QStringList* properties, vector<int>* nums)
{
	/// Check the inputs.
	if (properties == nullptr)
		return false;

	QStringList& props = *properties;
	props.clear();
	if (nums != nullptr)
		nums->clear();

	if (str_prop.empty())
		return true;

	/*stringstream ss(str_prop);
	string token;
	while (getline(ss, token, ','))
	{
	props.push_back(QString::fromStdString(token));
	}*/

	/// Split the string by ','.
	QString qProp = QString::fromStdString(str_prop);
	props = qProp.split(',');

	QString& last_prop = props.last();
	if (props.size() == 0
		|| str_prop.substr(0, 5) != "[0 ->"
		|| last_prop[last_prop.length() - 1] != ']')
		return false;

	props[0] = props[0].right(props[0].length() - 1);   // delete '[' at the beginning
	last_prop = last_prop.left(last_prop.length() - 1); // delete ']' at the end

														/// Exclude ',' which belong to property values.
														/// For all correct properties, detect the counting numbers and exclude them.
														// First property
	int num = -1;
	bool ok = recognize_property(&props[0], &num);
	if (ok && nums != nullptr)
		nums->push_back(num);
	bool isOK = ok;
	// All other properties
	auto it = props.begin();
	++it;
	while (it != props.end())
	{
		auto prev_it = it;
		--prev_it;
		auto& prop = *it;
		auto& prev_prop = *prev_it;

		int num = -1;
		bool ok = recognize_property(&prop, &num);
		if (ok)
		{
			if (nums != nullptr)
				nums->push_back(num);
			isOK = true;
			++it;
		}
		else
		{
			prev_prop = prev_prop + ',' + prop;
			it = props.erase(it);
			continue;
		}
	}

	assert(props.size() >= 1);
	assert(nums == nullptr || (props.size() == (int)nums->size()));
	return isOK;
}

size_t Elements::get_hash(mitk::BaseData* baseData)
{
	size_t retval = 0;
	if (baseData == nullptr)
		return retval;
	list<string> keys;
	string id = get_patientId(baseData, false);
	if (!id.empty()) keys.push_back(id);
	string name = get_patientName(baseData, false);
	if (!name.empty()) keys.push_back(name);
	string birthdate = get_patientBirthdate(baseData, false);
	if (!birthdate.empty()) keys.push_back(birthdate);
	string gender = get_patientGender(baseData, false);
	if (!gender.empty()) keys.push_back(gender);
	string imageName = get_imageName(baseData, false);
	if (!imageName.empty()) keys.push_back(imageName);
	vector<string> v_props = baseData->GetPropertyContextNames();
	list<string> props { make_move_iterator(v_props.begin()), make_move_iterator(v_props.end()) };
	keys.splice(keys.end(), props);
	string uid = baseData->GetUID();
	if (!uid.empty()) keys.push_back(uid);
	for (const auto& key : keys)
	{
		// retval = h1 ^ (h2 << 1)
		retval ^= hash<string>{}(key) + 0x9e3779b9 + (retval << 6) + (retval >> 2);
	}
	auto geo = baseData->GetGeometry();
	if (geo != nullptr)
	{
		auto center = geo->GetCenter();
		for (auto it = center.Begin(); it != center.End(); ++it)
		{
			auto& val = *it;
			retval ^= hash<double>{}(val)+0x9e3779b9 + (retval << 6) + (retval >> 2);
		}
	}
	return retval;
}
