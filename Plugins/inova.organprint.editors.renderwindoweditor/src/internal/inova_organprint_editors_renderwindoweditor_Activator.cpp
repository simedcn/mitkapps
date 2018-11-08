
#include "inova_organprint_editors_renderwindoweditor_Activator.h"

#include "OrganPrintRenderWindowEditor.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* inova_organprint_editors_renderwindoweditor_Activator::context;

void
inova_organprint_editors_renderwindoweditor_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(OrganPrintRenderWindowEditor, context)
}

void
inova_organprint_editors_renderwindoweditor_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

ctkPluginContext* inova_organprint_editors_renderwindoweditor_Activator::GetPluginContext()
{
	return context;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(inova_organprint_editors_renderwindoweditor, inova_organprint_editors_renderwindoweditor_Activator)
#endif
