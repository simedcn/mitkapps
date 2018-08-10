#include "TagTree.h"
#include <PopeElements.h>

#include <mitkDICOMTag.h>
//#include <mitkDICOMTagPath.h>

string change_prefix(const string& tag_name)
{
	string new_tag_name;
	const string dicom_image = "dicom.image.";
	string prefix = tag_name.substr(0, dicom_image.length());
	Elements::to_lower(prefix);
	if (prefix == dicom_image)
	{
		stringstream ss;
		ss << "DICOM." << tag_name.substr(dicom_image.length());
		new_tag_name = ss.str();
	}
	else
	{
		new_tag_name = tag_name;
	}
	return new_tag_name;
}
string find_description(const string& tag_name)
{
	string str_dicom = tag_name.substr(0, 6);
	Elements::to_upper(str_dicom);
	if (tag_name.length() == 15 && str_dicom == "DICOM." && tag_name[10] == '.')
	{
		try
		{
			unsigned int group = stoi(tag_name.substr(6, 4), nullptr, 16);
			unsigned int element = stoi(tag_name.substr(11, 4), nullptr, 16);
			mitk::DICOMTag dicom_tag(group, element);
			string description = dicom_tag.GetName();
			return description;
		}
		catch (...)
		{
		}
	}
	return "";
}
static const map<string, string> groups = 
{
	{ "DICOM.0002", "File MetaInformation" },
	{ "DICOM.0008", "Identifying" },
	{ "DICOM.0010", "Patient" },
	{ "DICOM.0018", "Acquisition" },
	{ "DICOM.0020", "Relationship" },
	{ "DICOM.0028", "Image Presentation" },
	{ "DICOM.0032", "Study" },
	{ "DICOM.4000", "Text" },
	{ "DICOM.6000", "Overlay" },
	{ "DICOM.7F",   "Pixel Data" }
};
string find_group_description(const string& group_name)
{
	string description;
	try
	{
		description = groups.at(group_name);
	}
	catch (const std::out_of_range&)
	{
		if (group_name.length() >= 10)
		{
			try
			{
				description = groups.at(group_name.substr(0, 8));
			}
			catch (const std::out_of_range&)
			{}
		}
	}
	return description;
}

Tag::Tag(const string& name, const string& value, const string& full_name)
	: name(name), value(value), full_name(full_name)
{
	//auto tag_path = mitk::PropertyNameToDICOMTagPath(tag.first);
	//string tag_name = mitk::DICOMTagPathToPropertyName(tag_path);

	string tag_name = change_prefix(full_name);
	description = find_description(tag_name);
	if (description.empty())
		description = name;
}

TagTree::TagTree(const string& full_name)
	: full_name(full_name)
{
	string tag_name = change_prefix(full_name);
	description = find_group_description(tag_name);
	if (description.empty())
		description = find_description(tag_name);
}

shared_ptr<Tag> ckeck_tree(TagNode tree, bool isRootNode)// = false)
{
	auto& groups = tree->tagGroups;
	auto& tags = tree->tags;

	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		auto& tag_group = *it;
		auto node = tag_group.second;
		auto tag_to_move = ckeck_tree(node, false);
		if (tag_to_move != nullptr)
		{
			const string& group_name = tag_group.first;
			stringstream new_tag_name;
			new_tag_name << group_name << '.' << tag_to_move->name;
			tag_to_move->name = new_tag_name.str();
			tags.push_back(tag_to_move);
			auto it_to_delete = it;
			--it;
			groups.erase(it_to_delete);
			if (it == groups.end())
			{// if the first element was deleted
				it = groups.begin();
				if (it == groups.end())
					break; // no more elements
			}
		}
	}
	if (!isRootNode && groups.size() == 0 && tags.size() == 1)
	{
		return tags.back();
	}
	return nullptr;
}
void correct_tree(TagNode tree)
{
	// Move single tags
	ckeck_tree(tree, true);
	//?? Move single groups
	// ?
}

shared_ptr<TagTree> TagTree::Create(const PropertyMap& property_map)
{
	auto root_tree = shared_ptr<TagTree>(new TagTree(""));

	/// Create a new tree
	for (const auto& tag : property_map)
	{
		string name = tag.first;
		string value = tag.second->GetValueAsString();
		TagNode tree = root_tree;
		stringstream prefix;
		while (true)
		{
			size_t pos = name.find('.', 1); // not at the first position
			if (pos == string::npos || pos == name.length() - 1)
			{// new tag
				stringstream full_name;
				full_name << prefix.str() << name;
				// Check the value: if it is an array, create a group
				QStringList elements;
				vector<int> nums;
				bool is_array = Elements::split_properties(value, &elements, &nums);
				if (is_array && nums.size() > 1 && elements.size() == nums.size())
				{
					assert(elements.size() == nums.size());
					string str_full_name = full_name.str();
					tree = tree->tagGroups[name] = shared_ptr<TagTree>(new TagTree(str_full_name));
					for (int i = 0; i < nums.size(); i++)
					{
						string name_i = to_string(nums[i]);
						stringstream full_name_i;
						full_name_i << str_full_name << '.' << name_i; //<< ".[" << name_i << ']';
						auto tag = shared_ptr<Tag>(new Tag(name_i, elements[i].toStdString(), full_name_i.str()));
						tree->tags.push_back(tag);
					}
				}
				else
				{
					auto tag = shared_ptr<Tag>(new Tag(name, value, full_name.str()));
					tree->tags.push_back(tag);
				}
				break;
			}
			else
			{// new or existing group
				string group_name = name.substr(0, pos);
				stringstream full_name;
				full_name << prefix.str() << group_name;
				auto new_group = tree->tagGroups[group_name];
				if (new_group == nullptr)
				{
					new_group = tree->tagGroups[group_name] = shared_ptr<TagTree>(new TagTree(full_name.str()));
				}
				tree = new_group;
				prefix << name.substr(0, pos + 1);
				name.erase(0, pos + 1);
			}
		}
	}

	/// Remove nodes with only one tag
	correct_tree(root_tree);

	return root_tree;
}