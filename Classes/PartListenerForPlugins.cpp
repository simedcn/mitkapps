
#include "PartListenerForPlugins.h"
#include "PluginDescriptors.h"

#include <berryUIException.h>
#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIProduct.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryFileEditorInput.h>

using namespace std;


PartListenerForPlugins::PartListenerForPlugins(berry::IPreferences::Pointer preferences) :
	preferences(preferences)
{}
berry::IPartListener::Events::Types PartListenerForPlugins::GetPartEventTypes() const
{
	return Events::OPENED | Events::CLOSED; //| Events::HIDDEN | Events::VISIBLE;
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

	/*const QString editor_id = "my.popeproject.editors.renderwindow";
	if (view_id == editor_id)
	{
		berry::IWorkbench* currentWorkbench = berry::PlatformUI::GetWorkbench();
		if (currentWorkbench)
		{
			berry::IWorkbenchWindow::Pointer currentWorkbenchWindow = currentWorkbench->GetActiveWorkbenchWindow();
			if (currentWorkbenchWindow)
			{
				berry::IEditorInput::Pointer editorInput2(new berry::FileEditorInput(QString()));
				currentWorkbenchWindow->GetActivePage()->OpenEditor(editorInput2, editor_id);
			}
		}
		return;
	}*/

	auto plugin = PluginDescriptors::find_plugin(view_id);
	if (plugin == nullptr)
		return;

	preferences->BlockSignals(true);
	preferences->PutBool(view_id, false);
	preferences->BlockSignals(false);
}
void PartListenerForPlugins::PartVisible(const berry::IWorkbenchPartReference::Pointer& ref)
{
	auto view_id = ref->GetId();
}
void PartListenerForPlugins::PartHidden(const berry::IWorkbenchPartReference::Pointer& ref)
{
	auto view_id = ref->GetId();
}
