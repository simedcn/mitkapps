
#include "inova_organprint_apps_minimalapplication_Activator.h"

#include "MinimalApplication.h"
#include "MinimalPerspective.h"
#include "ctkPluginContext.h"

ctkPluginContext* inova_organprint_apps_minimalapplication_Activator::m_Context = nullptr;

void inova_organprint_apps_minimalapplication_Activator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(MinimalApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(MinimalPerspective, context)

  m_Context = context;
}

void inova_organprint_apps_minimalapplication_Activator::stop(ctkPluginContext *context)
{
  Q_UNUSED(context)
  m_Context = nullptr;
}

ctkPluginContext *inova_organprint_apps_minimalapplication_Activator::GetContext()
{
  return m_Context;
}
