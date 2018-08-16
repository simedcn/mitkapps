#include "inova_popeproject_views_segmentation_PluginActivator.h"
#include "SegmentationView.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* inova_popeproject_views_segmentation_PluginActivator::context;

void inova_popeproject_views_segmentation_PluginActivator::start(ctkPluginContext* context)
{
	this->context = context;

	BERRY_REGISTER_EXTENSION_CLASS(SegmentationView, context)
}

void inova_popeproject_views_segmentation_PluginActivator::stop(ctkPluginContext*)
{
}

ctkPluginContext* inova_popeproject_views_segmentation_PluginActivator::GetPluginContext()
{
	return context;
}
