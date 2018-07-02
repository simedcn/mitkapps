#include <PopeElements.h>

#include "itkGDCMImageIO.h"

#include <sstream>
#include <algorithm>
#include <iterator>
#include <assert.h>


const vector<Elements::PluginDescriptor> Elements::plugins =
{
//	[order]		[id]										[name]						[role]			[title] [position]		[open]
    { "02",		"org.mitk.views.datamanager",				"Data Manager",				"main",			"yes",	"left",			"yes"	},
    { "01",		"my.popeproject.views.toolsplugin",			"POPE View",				"main",			"no",	"right",		"yes"	},
    { "06",		"org.mitk.views.basicimageprocessing",		"Image Processing",			"secondary", 	"no",	"right",		"no"	},
    { "12",		"my.popeproject.views.segmentation",		"Segmentation Tools",		"secondary",	"no",	"right",		"no"	},
    { "13",		"org.mitk.views.segmentation",				"Segmentation",				"secondary",	"no",	"right",		"no"	},
    { "14",		"org.mitk.views.segmentationutilities",		"Segmentation Utilities",	"secondary",	"no",	"right",		"no"	},
    { "15",		"org.mitk.views.deformableclippingplane",	"Clipping Plane",			"secondary",	"no",	"right",		"no"	},
    { "16",		"org.blueberry.views.logview",				"Log",						"secondary",	"no",	"right",		"no"	},
    { "03",		"org.mitk.views.imagenavigator",			"Image Navigator",			"secondary",	"no",	"bottom_right",	"yes"	},
    { "09",		"org.mitk.views.measurement",				"Measurement",				"secondary",	"no",	"bottom_right",	"no"	},
    { "10",		"org.mitk.views.imagecropper",				"Image Cropper",			"secondary",	"no",	"bottom_right",	"no"	},
    { "07",		"org.mitk.views.properties",				"Properties",				"secondary",	"no",	"bottom_left",	"no"	},
    { "08",		"org.mitk.views.volumevisualization",		"Volume Visualization",		"secondary",	"no",	"bottom_left",	"no"	},
    { "11",		"org.mitk.gui.qt.dicominspector",			"DICOM Inspector",			"secondary",	"no",	"bottom_left",	"no"	},
    { "04",		"my.pacs.views.dicomview",					"PACS Browser",				"secondary",	"yes",	"bottom", 		"no"	},
    { "05",		"org.mitk.views.xnat.treebrowser",			"XNAT Browser",				"secondary",	"yes",	"bottom", 		"no"	},
};
shared_ptr<vector<Elements::pPluginDescriptor>> Elements::plugins_in_order = nullptr;

Elements::PluginDescriptor::PluginDescriptor(initializer_list<QString> params)
{
    auto it = params.begin();
    QString str_i = *it++;
    bool ok = false;
    int num = str_i.toInt(&ok);
    this->order = ok ? (num - 1) : -1;

    this->id = *it++;
    this->name = *it++;

    const QString& role = *it++;
    this->is_main = (role.toLower() == "main");

    const QString& title_visibility = *it++;
    this->show_title = (title_visibility.toLower() == "yes");

    const QString plugin_position = (*it++).toLower();
    if (plugin_position == "bottom")
        this->position = PluginPosistion_bottom;
    else if (plugin_position == "bottom_left")
        this->position = PluginPosistion_bottom_left;
    else if (plugin_position == "bottom_right")
        this->position = PluginPosistion_bottom_right;
    else if (plugin_position == "left")
        this->position = PluginPosistion_left;
    else //if (plugin_position == "right")
        this->position = PluginPosistion_right;

    const QString& open = *it++;
    this->is_open = (open.toLower() == "yes");
}
/*const QString& Elements::PluginDescriptor::Id() const
{
	return id;
}
const QString& Elements::PluginDescriptor::Name() const
{
	return name;
}
bool Elements::PluginDescriptor::IsMain() const
{
	return is_main;
}
bool Elements::PluginDescriptor::IsTitleVisible() const
{
	return show_title;
}
Elements::PluginPosistion Elements::PluginDescriptor::Position() const
{
	return position;
}*/

Elements::Elements()
{}

void Elements::delete_spaces(string& str)
{
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
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

string Elements::get_property(const char* prop_a, const char* prop_b, const mitk::BaseData* data, const string& def_value)
{
    if (data == nullptr)
        return def_value;

    auto a = data->GetProperty(prop_a);
    if (a)
        return a->GetValueAsString();

    auto b = data->GetProperty(prop_b);
    if (b)
        return b->GetValueAsString();

    MITK_WARN << "Failed to read the DICOM tag " << prop_a << " (" << prop_b << ")";
    return def_value;
}
string Elements::get_property(const char* prop_a, const char* prop_b, const mitk::DataNode* dataNode, const string& def_value)
{
    if (dataNode == nullptr)
        return def_value;

    const auto data = dataNode->GetData();
    return get_property(prop_a, prop_b, data, def_value);
}
string Elements::get_patientId(mitk::BaseData* baseData, const string& def_value)
{
    return get_property("dicom.patient.PatientID", "DICOM.0010.0020", baseData, def_value);
}
string Elements::get_patientId(mitk::BaseData::Pointer baseData, const string& def_value)
{
    if (baseData == nullptr)
        return def_value;

    return get_patientId(baseData.GetPointer(), def_value);
}
string Elements::get_patientId(mitk::DataNode* dataNode, const string& def_value)
{
    if (dataNode == nullptr)
        return def_value;

    const auto data = dataNode->GetData();
    return get_patientId(data, def_value);
}
string Elements::get_patientId_or_patientName(mitk::BaseData* baseData, const string& def_value)
{
    string id = get_patientId(baseData, "");
    if (!id.empty())
        return id;

    id = get_patientName(baseData, "");
    if (!id.empty())
        return id;

    return def_value;
}
string Elements::get_patientId_or_patientName(mitk::BaseData::Pointer baseData, const string& def_value)
{
    if (baseData == nullptr)
        return def_value;

    return get_patientId_or_patientName(baseData.GetPointer(), def_value);
}
string Elements::get_patientId_or_patientName(mitk::DataNode* dataNode, const string& def_value)
{
    if (dataNode == nullptr)
        return def_value;

    const auto data = dataNode->GetData();
    return get_patientId_or_patientName(data, def_value);
}
QString Elements::get_patientId_or_patientName(const string& filename, const QString& def_value)
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
    MITK_WARN << "No patientID in " << filename;
    return def_value;
}
QString Elements::get_patientId_or_patientName(const QString& filename, const QString& def_value)
{
    string name = filename.toStdString();
    return get_patientId_or_patientName(name, def_value);
}
string Elements::get_patientName(mitk::BaseData* baseData, const string& def_value)
{
    return get_property("dicom.patient.PatientsName", "DICOM.0010.0010", baseData, def_value);
}
string Elements::get_patientName(mitk::BaseData::Pointer baseData, const string& def_value)
{
    if (baseData == nullptr)
        return def_value;

    return get_patientName(baseData.GetPointer(), def_value);
}
string Elements::get_patientName(mitk::DataNode* dataNode, const string& def_value)
{
    if (dataNode == nullptr)
        return def_value;

    const auto data = dataNode->GetData();
    return get_patientName(data, def_value);
}
string Elements::find_patientName(const list<mitk::BaseData::Pointer>& baseDataList, const string& def_value)
{
    if (baseDataList.size() == 0)
        return "";

    const char* prop_a = "DICOM.0010.0010";
    const char* prop_b = "dicom.patient.PatientsName";

    // Read all the data sets until the patient name is found
    for (const auto baseData : baseDataList)
    {
        if (baseData == nullptr || baseData.IsNull())
            continue;

        auto a = baseData->GetProperty(prop_a);
        if (a)
            return a->GetValueAsString();

        auto b = baseData->GetProperty(prop_b);
        if (b)
            return b->GetValueAsString();
    }

    MITK_WARN << "Failed to read the DICOM tag " << prop_a << " (" << prop_b << ")";
    return def_value;
}
string Elements::get_patientBirthdate(mitk::BaseData* baseData, const string& def_value)
{
    return get_property("dicom.patient.PatientsBirthDate", "DICOM.0010.0030", baseData, def_value);
}
string Elements::get_patientBirthdate(mitk::BaseData::Pointer baseData, const string& def_value)
{
    if (baseData == nullptr)
        return def_value;

    return get_patientBirthdate(baseData.GetPointer(), def_value);
}
string Elements::get_patientBirthdate(mitk::DataNode* dataNode, const string& def_value)
{
    if (dataNode == nullptr)
        return def_value;

    const auto data = dataNode->GetData();
    return get_patientBirthdate(data, def_value);
}
string Elements::get_patientGender(mitk::BaseData* baseData, const string& def_value)
{
    return get_property("dicom.patient.PatientsSex", "DICOM.0010.0040", baseData, def_value);
}
string Elements::get_patientGender(mitk::BaseData::Pointer baseData, const string& def_value)
{
    if (baseData == nullptr)
        return def_value;

    return get_patientGender(baseData.GetPointer(), def_value);
}
string Elements::get_patientGender(mitk::DataNode* dataNode, const string& def_value)
{
    if (dataNode == nullptr)
        return def_value;

    const auto data = dataNode->GetData();
    return get_patientGender(data, def_value);
}
string Elements::get_imageName(mitk::BaseData* baseData, const string& def_value)
{
    if (baseData == nullptr)
        return def_value;

    stringstream imageName;
    auto prop_SeriesInstanceUID = baseData->GetProperty("dicom.series.SeriesInstanceUID");
    if (prop_SeriesInstanceUID)
        imageName << prop_SeriesInstanceUID->GetValueAsString();
    else
        imageName << "image";
    auto prop_Modality = baseData->GetProperty("dicom.series.Modality");
    if (prop_Modality)
    {
        imageName << "_";
        imageName << prop_Modality->GetValueAsString();
    }
    return imageName.str();
}
string Elements::get_imageName(mitk::BaseData::Pointer baseData, const string& def_value)
{
    if (baseData == nullptr)
        return def_value;

    return get_imageName(baseData.GetPointer(), def_value);
}
string Elements::get_imageName(mitk::DataNode* dataNode, const string& def_value)
{
    if (dataNode == nullptr)
        return def_value;

    const auto data = dataNode->GetData();
    return get_imageName(data, def_value);
}
vector<QString> Elements::get_imageNames(const list<mitk::BaseData::Pointer>& baseDataList, const string& def_value)
{
    // Use std::set to allow only unique image names
    set<QString> imageSet;
    for (const auto baseData : baseDataList)
    {
        string name = get_imageName(baseData, def_value);
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
    assert(nums == nullptr || (props.size() == (signed) nums->size()));
    return isOK;
}

shared_ptr<vector<Elements::pPluginDescriptor>> Elements::plugins_by_order()
{
    if (plugins_in_order == nullptr)
    {
        // Copy the elements
        plugins_in_order = make_shared<vector<Elements::pPluginDescriptor>>(plugins.size());
        for (uint i = 0; i < plugins.size(); i++)
        {
            plugins_in_order->operator[](i) = &plugins[i];
        }
        // Sort the elements by their order
        sort(plugins_in_order->begin(), plugins_in_order->end(),
             [](Elements::pPluginDescriptor a, Elements::pPluginDescriptor b)
        {
            return (a->order < b->order);
        }
            );
    }
    return plugins_in_order;
}
const Elements::PluginDescriptor* Elements::find_plugin(const QString& id)
{
    for (const auto& plugin : Elements::plugins)
    {
        if (plugin.id == id)
        {
            return &plugin;
        }
    }
    return nullptr;
}
