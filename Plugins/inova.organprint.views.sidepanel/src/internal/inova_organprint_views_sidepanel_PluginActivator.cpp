
#include "inova_organprint_views_sidepanel_PluginActivator.h"
//#include "OrganPrintView.h"
#include "ImportPanel.h"
#include "ExportPanel.h"
#include "TissuePanel.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext * inova_organprint_views_sidepanel_PluginActivator::context;

void inova_organprint_views_sidepanel_PluginActivator::start(ctkPluginContext* context)
{
    //BERRY_REGISTER_EXTENSION_CLASS(OrganPrintView, context);

    BERRY_REGISTER_EXTENSION_CLASS(orgpnt::TissuePanel, context);
    BERRY_REGISTER_EXTENSION_CLASS(orgpnt::ImportPanel, context);
    BERRY_REGISTER_EXTENSION_CLASS(orgpnt::ExportPanel, context);

    this->context = context;
}

void inova_organprint_views_sidepanel_PluginActivator::stop(ctkPluginContext*)
{
}

ctkPluginContext * inova_organprint_views_sidepanel_PluginActivator::GetPluginContext()
{
    return context;
}
