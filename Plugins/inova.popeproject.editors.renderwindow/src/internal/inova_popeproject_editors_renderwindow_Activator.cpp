#include "inova_popeproject_editors_renderwindow_Activator.h"
#include "PopeRenderWindowEditor.h"
#include "PopePreferencePage.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* inova_popeproject_editors_renderwindow_Activator::context;


void inova_popeproject_editors_renderwindow_Activator::start(ctkPluginContext* context)
{
	this->context = context;

	BERRY_REGISTER_EXTENSION_CLASS(PopeRenderWindowEditor, context)
	BERRY_REGISTER_EXTENSION_CLASS(PopePreferencePage, context)
}

void inova_popeproject_editors_renderwindow_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

ctkPluginContext* inova_popeproject_editors_renderwindow_Activator::GetPluginContext()
{
	return context;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(inova_popeproject_editors_renderwindow, inova_popeproject_editors_renderwindow_Activator)
#endif
