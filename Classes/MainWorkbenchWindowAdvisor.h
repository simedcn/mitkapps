#ifndef MAINWORKBENCHWINDOWADVISOR_H_
#define MAINWORKBENCHWINDOWADVISOR_H_

#include "PluginDescriptors.h"
#include "ShowViewAction.h"

#include <QmitkExtWorkbenchWindowAdvisor.h>

#include <berryWorkbenchPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryIViewDescriptor.h>
#include <berryIPreferences.h>
#include <berryIBerryPreferences.h>

#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>
#include <QToolBar>
#include <QToolButton>

#include <memory>
#include <vector>

using namespace std;


class MainWorkbenchWindowAdvisor : public QmitkExtWorkbenchWindowAdvisor
{
	Q_OBJECT

public:
	MainWorkbenchWindowAdvisor(berry::WorkbenchAdvisor* wbAdvisor, berry::IWorkbenchWindowConfigurer::Pointer configurer, const initializer_list<PluginDescriptor>& plugins, const QString& preferencesNode, bool addProjectLabel = false);
	~MainWorkbenchWindowAdvisor();

public:
	void PostWindowCreate() override;
	void PostWindowRestore() override;
	bool PreWindowShellClose() override;

protected:
	QString ComputeTitle() override;
	void OnPreferencesChanged(const berry::IBerryPreferences*);

protected:
	void setPluginTitle(const PluginDescriptor& plugin, berry::IWorkbenchPage::Pointer page = berry::IWorkbenchPage::Pointer());
	void setPluginTitle(pViewInfo viewInfo, berry::IWorkbenchPage::Pointer page = berry::IWorkbenchPage::Pointer());
	ViewDescriptors createViewDescriptors();
	void replaceViewActions(berry::IWorkbenchWindow::Pointer window, ViewDescriptors viewDescriptors);
	void createViewMenu();
	void updateMainToolbar();
	void updateViewToolbar();
	void createViewToolbar(berry::IWorkbenchWindow::Pointer window, ViewDescriptors viewDescriptors);
	void findSubMenus(QMenu** windowMenu, QMenu** editMenu);
	void setMenuPreferencesItem(QMenu* windowMenu, QMenu* editMenu);
	void removeRedoAndUndo(QMenu* editMenu);
	void removeImageNavigatorToggle();
	void manageViews(berry::IWorkbenchPage::Pointer page, bool toConfigureTitles = true);
	void maximizeWindow(berry::IWorkbenchWindow::Pointer window);

protected slots:
	void on_tabsToolButton_triggered();

protected:
	QScopedPointer<berry::IPartListener> viewPartListener;
	bool is_restored = false;
	berry::IPreferences::Pointer mainApplicationPreferencesNode;
	QMainWindow* mainWindow = nullptr;
	QToolBar* viewToolbar = nullptr;
	QToolButton* tabsToolButton = nullptr;
	QList<QAction*> viewToolbarActions;

public:
	bool addProjectLabel = false;
	bool addShowTabsButton = true;
};

#endif /*MAINWORKBENCHWINDOWADVISOR_H_*/
