#include "TagTree.h"

#include <PopeElements.h>

Tag::Tag(const string& name, const string& value)
	: name(name), value(value)
{}

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
	ckeck_tree(tree, true);
}

shared_ptr<TagTree> TagTree::Create(const PropertyMap& property_map)
{
	auto root_tree = shared_ptr<TagTree>(new TagTree);

	/// Create a new tree
	for (const auto& tag : property_map)
	{
		string name = tag.first;
		string value = tag.second->GetValueAsString();
		TagNode tree = root_tree;
		while (true)
		{
			size_t pos = name.find('.', 1); // not at the first position
			if (pos == string::npos || pos == name.length() - 1)
			{// new tag
				// Check the value: if it is an array, create a group
				QStringList elements;
				vector<int> nums;
				bool is_array = Elements::split_properties(value, &elements, &nums);
				if (is_array && nums.size() > 1 && elements.size() == nums.size())
				{
					assert(elements.size() == nums.size());
					tree = tree->tagGroups[name] = shared_ptr<TagTree>(new TagTree);
					for (int i = 0; i < nums.size(); i++)
					{
						auto tag = shared_ptr<Tag>(new Tag(to_string(nums[i]), elements[i].toStdString()));
						tree->tags.push_back(tag);
					}
				}
				else
				{
					auto tag = shared_ptr<Tag>(new Tag(name, value));
					tree->tags.push_back(tag);
				}
				break;
			}
			else
			{// new or existing group
				string group_name = name.substr(0, pos);
				auto new_group = tree->tagGroups[group_name];
				if (new_group == nullptr)
				{
					new_group = tree->tagGroups[group_name] = shared_ptr<TagTree>(new TagTree);
				}
				tree = new_group;
				name.erase(0, pos + 1);
			}
		}
	}

	/// Remove nodes with only one tag
	correct_tree(root_tree);

	return root_tree;
}