#include "my_popeproject_registrationplugin_PluginActivator.h"
#include "RegistrationPlugin.h"
#include "RegistrationPluginPreferencePage.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* my_popeproject_registrationplugin_PluginActivator::context;

void my_popeproject_registrationplugin_PluginActivator::start(ctkPluginContext* context)
{
	this->context = context;

	BERRY_REGISTER_EXTENSION_CLASS(RegistrationPlugin, context)
	BERRY_REGISTER_EXTENSION_CLASS(RegistrationPluginPreferencePage, context)
}

void my_popeproject_registrationplugin_PluginActivator::stop(ctkPluginContext*)
{
}

ctkPluginContext* my_popeproject_registrationplugin_PluginActivator::GetPluginContext()
{
	return context;
}