
#include "inova_organprint_views_stepselector_PluginActivator.h"
#include "StepSelector.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext * inova_organprint_views_stepselector_PluginActivator::context;

void inova_organprint_views_stepselector_PluginActivator::start(ctkPluginContext* context)
{
    BERRY_REGISTER_EXTENSION_CLASS(StepSelector, context);
    this->context = context;

}

void inova_organprint_views_stepselector_PluginActivator::stop(ctkPluginContext*)
{
}
ctkPluginContext * inova_organprint_views_stepselector_PluginActivator::GetPluginContext()
{
    return context;
}
