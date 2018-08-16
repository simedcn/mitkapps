#ifndef inova_popeproject_views_tools_PluginActivator_h
#define inova_popeproject_views_tools_PluginActivator_h

#include <ctkPluginActivator.h>

class inova_popeproject_views_tools_PluginActivator
	: public QObject
	, public ctkPluginActivator
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "inova_popeproject_views_tools")
	Q_INTERFACES(ctkPluginActivator)

public:
	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static ctkPluginContext* GetPluginContext();

private:
	static ctkPluginContext* context;
};

#endif
