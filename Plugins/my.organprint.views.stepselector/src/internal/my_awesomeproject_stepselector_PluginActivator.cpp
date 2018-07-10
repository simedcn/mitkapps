
#include "my_awesomeproject_stepselector_PluginActivator.h"
#include "StepSelector.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* my_awesomeproject_stepselector_PluginActivator::context;

void my_awesomeproject_stepselector_PluginActivator::start(ctkPluginContext* context)
{
  my_awesomeproject_stepselector_PluginActivator::context = context;

  BERRY_REGISTER_EXTENSION_CLASS(StepSelector, context)
}

void my_awesomeproject_stepselector_PluginActivator::stop(ctkPluginContext*)
{
}

ctkPluginContext* my_awesomeproject_stepselector_PluginActivator::GetPluginContext()
{
  return context;
}