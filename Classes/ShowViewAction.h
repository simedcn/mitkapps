#ifndef SHOWVIEWACTIONS_H_
#define SHOWVIEWACTIONS_H_

#include "PluginDescriptors.h"

#include <berryWorkbenchPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryIViewDescriptor.h>

#include <QAction>

#include <memory>
#include <vector>

using namespace std;

struct ViewInfo;
using pViewInfo = shared_ptr<const ViewInfo>;
using ViewDescriptors = shared_ptr<vector<pViewInfo>>;


struct ViewInfo
{
	ViewInfo(berry::IViewDescriptor::Pointer);

	QString Id() const;
	QString Name() const;
	int Order() const;
	void ConfigureTitle(berry::IViewPart::Pointer view) const;

	berry::IViewDescriptor::Pointer view_desc;
	const PluginDescriptor* plugin_desc;
};


class ShowViewAction : public QAction
{
	Q_OBJECT
protected:
	berry::IWorkbenchWindow* m_Window;
	pViewInfo m_Info;

public:
	ShowViewAction(berry::IWorkbenchWindow::Pointer window, pViewInfo viewInfo);

	protected slots:
	void Run();
};

#endif /*SHOWVIEWACTIONS_H_*/
