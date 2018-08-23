#include "PluginListener.h"
#include "inova_popeproject_apps_mainapplication_Activator.h"

#include <berryUIException.h>
#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIProduct.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryFileEditorInput.h>

#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEventConstants.h>
#include <ctkServiceReference.h>

#include <assert.h>

using namespace std;


PluginListener::PluginListener()
{
	/// CTK signals.
	auto pluginContext = inova_popeproject_apps_mainapplication_Activator::GetContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	assert(ref);
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		eventAdmin->publishSignal(this, SIGNAL(PluginVisible(const ctkDictionary&)), "plugin/VISIBLE", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(PluginHidden(const ctkDictionary&)), "plugin/HIDDEN", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(PluginDeactivated(const ctkDictionary&)), "plugin/DEACTIVATED", Qt::DirectConnection);
	}
}
berry::IPartListener::Events::Types PluginListener::GetPartEventTypes() const
{
	return Events::VISIBLE | Events::HIDDEN;// | Events::OPENED | Events::CLOSED | Events::ACTIVATED | Events::DEACTIVATED;
}
void PluginListener::PartOpened(const berry::IWorkbenchPartReference::Pointer& ref)
{
	QString view_id = ref->GetId();
}
void PluginListener::PartClosed(const berry::IWorkbenchPartReference::Pointer& ref)
{
	QString view_id = ref->GetId();
}
void PluginListener::PartActivated(const berry::IWorkbenchPartReference::Pointer& ref)
{
	QString view_id = ref->GetId();
}
void PluginListener::PartDeactivated(const berry::IWorkbenchPartReference::Pointer& ref)
{
	QString view_id = ref->GetId();
	ctkDictionary properties;
	properties["id"] = view_id;
	emit PluginDeactivated(properties);
}
void PluginListener::PartVisible(const berry::IWorkbenchPartReference::Pointer& ref)
{
	QString view_id = ref->GetId();
	ctkDictionary properties;
	properties["id"] = view_id;
	emit PluginVisible(properties);
}
void PluginListener::PartHidden(const berry::IWorkbenchPartReference::Pointer& ref)
{
	QString view_id = ref->GetId();
	ctkDictionary properties;
	properties["id"] = view_id;
	emit PluginHidden(properties);
}
