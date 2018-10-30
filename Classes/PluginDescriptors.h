#ifndef PLUGINDESCRIPTOR_H_
#define PLUGINDESCRIPTOR_H_

#include <QString>

#include <memory>
#include <vector>
#include <initializer_list>

using namespace std;


struct PluginDescriptor
{
public:
	enum PluginPosistion
	{
		PluginPosistion_top_left,
		PluginPosistion_mid_left,
		PluginPosistion_bottom_left,
		PluginPosistion_right,
		//PluginPosistion_bottom_right,
		PluginPosistion_bottom,
	};
	enum PluginRole
	{
		PluginRole_main,
		PluginRole_secondary,
		PluginRole_pacs,
		PluginRole_selector,
		PluginRole_selectorItem
	};

public:
	PluginDescriptor(initializer_list<QString> params);

	//protected:
public:
	int order = -1;
	QString id;
	QString name;
	PluginRole role = PluginRole::PluginRole_secondary;
	bool show_title = false;
	PluginPosistion position = PluginPosistion_right;
	bool is_open = true;

public:
	//	const QString& Id() const;
	//	const QString& Name() const;
	//	bool IsMain() const;
	//	bool IsTitleVisible() const;
	//	PluginPosistion Position() const;
};

class PluginDescriptors
{
public:
	using pPluginDescriptor = const PluginDescriptor*;

private:
	PluginDescriptors();

public:
	static void set(const initializer_list<PluginDescriptor>& plugins);
	static const vector<PluginDescriptor>& get();
	static shared_ptr<vector<pPluginDescriptor>> plugins_by_order();
	static pPluginDescriptor find_plugin(const QString& id);

protected:
	static vector<PluginDescriptor> plugins;
	static shared_ptr<vector<pPluginDescriptor>> plugins_in_order;

};
#endif /*PLUGINDESCRIPTOR_H_*/
