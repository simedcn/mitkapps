
#include "PartListenerForPlugins.h"
#include "PluginDescriptors.h"

#include <berryUIException.h>
#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIProduct.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>

using namespace std;


PartListenerForPlugins::PartListenerForPlugins(berry::IPreferences::Pointer preferences) :
	preferences(preferences)
{}
berry::IPartListener::Events::Types PartListenerForPlugins::GetPartEventTypes() const
{
	return Events::OPENED | Events::CLOSED;// | Events::HIDDEN | Events::VISIBLE;
}
void PartListenerForPlugins::PartOpened(const berry::IWorkbenchPartReference::Pointer& ref)
{
	auto view_id = ref->GetId();
	auto plugin = PluginDescriptors::find_plugin(view_id);
	if (plugin == nullptr)
		return;

	preferences->BlockSignals(true);
	preferences->PutBool(view_id, true);
	preferences->BlockSignals(false);
}
void PartListenerForPlugins::PartClosed(const berry::IWorkbenchPartReference::Pointer& ref)
{
	auto view_id = ref->GetId();
	auto plugin = PluginDescriptors::find_plugin(view_id);
	if (plugin == nullptr)
		return;

	preferences->BlockSignals(true);
	preferences->PutBool(view_id, false);
	preferences->BlockSignals(false);
}
void PartListenerForPlugins::PartVisible(const berry::IWorkbenchPartReference::Pointer& ref)
{}
void PartListenerForPlugins::PartHidden(const berry::IWorkbenchPartReference::Pointer& ref)
{}
