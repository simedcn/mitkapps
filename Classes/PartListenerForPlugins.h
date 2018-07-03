#ifndef PARTLISTENERFORPLUGINS_H_
#define PARTLISTENERFORPLUGINS_H_

#include <berryWorkbenchPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryIViewDescriptor.h>
#include <berryIPreferences.h>
#include <berryIBerryPreferences.h>

class PartListenerForPlugins : public berry::IPartListener
{
public:
	PartListenerForPlugins(berry::IPreferences::Pointer preferences);

	Events::Types GetPartEventTypes() const override;
	void PartOpened(const berry::IWorkbenchPartReference::Pointer& ref) override;
	void PartClosed(const berry::IWorkbenchPartReference::Pointer& ref) override;
	void PartVisible(const berry::IWorkbenchPartReference::Pointer& ref) override;
	void PartHidden(const berry::IWorkbenchPartReference::Pointer& ref) override;

protected:
	berry::IPreferences::Pointer preferences;
};
#endif /*PARTLISTENERFORPLUGINS_H_*/
