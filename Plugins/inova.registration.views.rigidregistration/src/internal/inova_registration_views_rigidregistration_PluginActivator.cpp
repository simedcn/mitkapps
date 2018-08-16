#include "inova_registration_views_rigidregistration_PluginActivator.h"
#include "RegistrationPlugin.h"
#include "RegistrationPluginPreferencePage.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* inova_registration_views_rigidregistration_PluginActivator::context;

void inova_registration_views_rigidregistration_PluginActivator::start(ctkPluginContext* context)
{
	this->context = context;

	BERRY_REGISTER_EXTENSION_CLASS(RegistrationPlugin, context)
	BERRY_REGISTER_EXTENSION_CLASS(RegistrationPluginPreferencePage, context)
}

void inova_registration_views_rigidregistration_PluginActivator::stop(ctkPluginContext*)
{
}

ctkPluginContext* inova_registration_views_rigidregistration_PluginActivator::GetPluginContext()
{
	return context;
}