
#ifndef WindowAndPerspectiveListener_H_
#define WindowAndPerspectiveListener_H_

#include <ctkPluginActivator.h>

#include <service/event/ctkEvent.h>
#include <service/event/ctkEventHandler.h>

#include <QScopedPointer>
#include <QMutex>

#include <berryIWorkbenchPage.h>
#include <berryIWindowListener.h>

using namespace berry;

class PerspectiveListener : public IPerspectiveListener
{
public:

	Events::Types GetPerspectiveEventTypes() const override;

	using IPerspectiveListener::PerspectiveChanged;

	void PerspectiveOpened(const SmartPointer<IWorkbenchPage>& page, const IPerspectiveDescriptor::Pointer& perspective) override;
	void PerspectiveChanged(const SmartPointer<IWorkbenchPage>& page, const IPerspectiveDescriptor::Pointer& perspective, const QString &changeId) override;

public:
	static const QString EDITOR_ID;
};

class WindowListener : public IWindowListener
{
public:

	WindowListener();
	~WindowListener();

	void WindowClosed(const IWorkbenchWindow::Pointer& window) override;
	void WindowOpened(const IWorkbenchWindow::Pointer& window) override;

private:

	// We use the same perspective listener for every window
	QScopedPointer<IPerspectiveListener> perspListener;
};


#endif /*WindowAndPerspectiveListener_H_*/
