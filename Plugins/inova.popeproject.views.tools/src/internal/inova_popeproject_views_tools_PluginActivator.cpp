#include "inova_popeproject_views_tools_PluginActivator.h"
#include "ToolsPlugin.h"
#include "ToolsPluginPreferencePage.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* inova_popeproject_views_tools_PluginActivator::context;

void inova_popeproject_views_tools_PluginActivator::start(ctkPluginContext* context)
{
	this->context = context;

	BERRY_REGISTER_EXTENSION_CLASS(ToolsPlugin, context)
	BERRY_REGISTER_EXTENSION_CLASS(ToolsPluginPreferencePage, context)
}

void inova_popeproject_views_tools_PluginActivator::stop(ctkPluginContext*)
{
}

ctkPluginContext* inova_popeproject_views_tools_PluginActivator::GetPluginContext()
{
	return context;
}