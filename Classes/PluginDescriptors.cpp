#include "PluginDescriptors.h"

// PluginDescriptors

vector<PluginDescriptor> PluginDescriptors::plugins;
shared_ptr<vector<PluginDescriptors::pPluginDescriptor>> PluginDescriptors::plugins_in_order = nullptr;

PluginDescriptors::PluginDescriptors()
{}
void PluginDescriptors::set(const initializer_list<PluginDescriptor>& plugins)
{
	PluginDescriptors::plugins = plugins;
}
const vector<PluginDescriptor>& PluginDescriptors::get()
{
	return plugins;
}
shared_ptr<vector<PluginDescriptors::pPluginDescriptor>> PluginDescriptors::plugins_by_order()
{
	if (plugins_in_order == nullptr)
	{
		// Copy the elements
		plugins_in_order = make_shared<vector<pPluginDescriptor>>(plugins.size());
		for (int i = 0; i < plugins.size(); i++)
		{
			plugins_in_order->operator[](i) = &plugins[i];
		}
		// Sort the elements by their order
		sort(plugins_in_order->begin(), plugins_in_order->end(),
			[](pPluginDescriptor a, pPluginDescriptor b)
			{
				return (a->order < b->order);
			}
		);
	}
	return plugins_in_order;
}
const PluginDescriptor* PluginDescriptors::find_plugin(const QString& id)
{
	for (const auto& plugin : plugins)
	{
		if (plugin.id == id)
		{
			return &plugin;
		}
	}
	return nullptr;
}

// PluginDescriptor
PluginDescriptor::PluginDescriptor(initializer_list<QString> params)
{
	//	[order] [id] [name] [role] [title] [position] [open]
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
/*const QString& PluginDescriptor::Id() const
{
return id;
}
const QString& PluginDescriptor::Name() const
{
return name;
}
bool PluginDescriptor::IsMain() const
{
return is_main;
}
bool PluginDescriptor::IsTitleVisible() const
{
return show_title;
}
PluginPosistion PluginDescriptor::Position() const
{
return position;
}*/
