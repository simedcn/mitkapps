#include "inova_registration_views_stepselector_PluginActivator.h"
#include "RegistrationStepSelector.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* inova_registration_views_stepselector_PluginActivator::context;

void inova_registration_views_stepselector_PluginActivator::start(ctkPluginContext* context)
{
	this->context = context;

	BERRY_REGISTER_EXTENSION_CLASS(RegistrationStepSelector, context)
}

void inova_registration_views_stepselector_PluginActivator::stop(ctkPluginContext*)
{
}

ctkPluginContext* inova_registration_views_stepselector_PluginActivator::GetPluginContext()
{
	return context;
}