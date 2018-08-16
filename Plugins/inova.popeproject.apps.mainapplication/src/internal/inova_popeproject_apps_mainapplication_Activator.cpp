#include "inova_popeproject_apps_mainapplication_Activator.h"

#include "MainApplication.h"
#include "MainPerspective.h"
#include "DicomPerspective.h"
#include "MainApplicationPreferencePage.h"

#include <ctkPluginContext.h>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

ctkPluginContext* inova_popeproject_apps_mainapplication_Activator::m_Context = nullptr;

void inova_popeproject_apps_mainapplication_Activator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(MainApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(MainPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(DicomPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(MainApplicationPreferencePage, context)

  ////BERRY_REGISTER_EXTENSION_CLASS(QmitkDicomBrowser, context);
  //auto prefService = berry::Platform::GetPreferencesService();
  //berry::IPreferences::Pointer prefNodesWithNoData = prefService->GetSystemPreferences()->Node("/org.mitk.editors.dicombrowser");
  //prefNodesWithNoData->PutBool("Show nodes containing no data", true);



  m_Context = context;
}

void inova_popeproject_apps_mainapplication_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  m_Context = nullptr;
}

ctkPluginContext* inova_popeproject_apps_mainapplication_Activator::GetContext()
{
  return m_Context;
}
