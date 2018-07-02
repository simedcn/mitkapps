
#include "my_awesomeproject_stepselector_PluginActivator.h"
#include "StepSelector.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

void my_awesomeproject_stepselector_PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(StepSelector, context)
}

void my_awesomeproject_stepselector_PluginActivator::stop(ctkPluginContext*)
{
}
