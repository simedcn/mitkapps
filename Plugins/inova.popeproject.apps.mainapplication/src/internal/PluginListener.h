#ifndef PluginListener_H_
#define PluginListener_H_

#include <berryWorkbenchPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryIViewDescriptor.h>
#include <berryIPreferences.h>
#include <berryIBerryPreferences.h>

#include <ctkDictionary.h>


class PluginListener : public QObject, public berry::IPartListener
{
	Q_OBJECT

public:
	PluginListener();

	Events::Types GetPartEventTypes() const override;
	void PartOpened(const berry::IWorkbenchPartReference::Pointer& ref) override;
	void PartClosed(const berry::IWorkbenchPartReference::Pointer& ref) override;
	void PartVisible(const berry::IWorkbenchPartReference::Pointer& ref) override;
	void PartHidden(const berry::IWorkbenchPartReference::Pointer& ref) override;
	void PartActivated(const berry::IWorkbenchPartReference::Pointer& ref) override;
	void PartDeactivated(const berry::IWorkbenchPartReference::Pointer& ref) override;

signals:
	void PluginVisible(const ctkDictionary&);
	void PluginHidden(const ctkDictionary&);
	void PluginDeactivated(const ctkDictionary&);
};
#endif /*PluginListener_H_*/
