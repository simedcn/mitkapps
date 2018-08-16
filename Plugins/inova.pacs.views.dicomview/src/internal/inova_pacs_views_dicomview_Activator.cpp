#include "inova_pacs_views_dicomview_Activator.h"

#include "DicomView.h"
#include "DicomViewPreferencePage.h"
#include "WindowListener.h"
#include "UploadToPACSAction.h"

#include <berryIWorkbenchPage.h>
#include <berryPlatformUI.h>

ctkPluginContext* inova_pacs_views_dicomview_Activator::PluginContext = nullptr;

void inova_pacs_views_dicomview_Activator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(DicomView, context)
  BERRY_REGISTER_EXTENSION_CLASS(DicomViewPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(UploadToPACSAction, context)
  PluginContext = context;

  // Register a window listener which registers a perspective listener for each
  // new window. The perspective listener opens the help home page in the window
  // if no other help page is opened yet.
//  wndListener.reset(new WindowListener());
//  berry::PlatformUI::GetWorkbench()->AddWindowListener(wndListener.data());
}

void inova_pacs_views_dicomview_Activator::stop(ctkPluginContext *context)
{
  Q_UNUSED(context)

  PluginContext = nullptr;
}

ctkPluginContext* inova_pacs_views_dicomview_Activator::GetPluginContext()
{
  return PluginContext;
}
