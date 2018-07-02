#ifndef MAINWORKBENCHWINDOWADVISOR_H_
#define MAINWORKBENCHWINDOWADVISOR_H_

#include <PopeElements.h>
#include "ShowViewAction.h"
#include "PartListenerForPlugins.h"

#include "QmitkExtWorkbenchWindowAdvisor.h"

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

#include <memory>
#include <vector>

using namespace std;


class MainWorkbenchWindowAdvisor : public QmitkExtWorkbenchWindowAdvisor
{
public:
	MainWorkbenchWindowAdvisor(berry::WorkbenchAdvisor* wbAdvisor, berry::IWorkbenchWindowConfigurer::Pointer configurer);
	~MainWorkbenchWindowAdvisor();

public:
	void PostWindowCreate() override;
	bool PreWindowShellClose() override;

protected:
	QString ComputeTitle() override;
	void OnPreferencesChanged(const berry::IBerryPreferences*);

protected:
	void setPluginTitle(const Elements::PluginDescriptor& plugin, berry::IWorkbenchPage::Pointer page = berry::IWorkbenchPage::Pointer());
	void setPluginTitle(pViewInfo viewInfo, berry::IWorkbenchPage::Pointer page = berry::IWorkbenchPage::Pointer());
	ViewDescriptors MainWorkbenchWindowAdvisor::createViewDescriptors();
	void replaceViewActions(berry::IWorkbenchWindow::Pointer window, ViewDescriptors viewDescriptors);
	void createViewMenu(QMainWindow* mainWindow);
	void createViewToolbar(berry::IWorkbenchWindow::Pointer window, QMainWindow* mainWindow, ViewDescriptors viewDescriptors);
	void findSubMenus(QMainWindow* mainWindow, QMenu** windowMenu, QMenu** editMenu);
	void setMenuPreferencesItem(QMenu* windowMenu, QMenu* editMenu);
	void removeRedoAndUndo(QMainWindow* mainWindow, QMenu* editMenu);
	void manageViews(berry::IWorkbenchPage::Pointer page, berry::IPreferences::Pointer mainApplicationPreferencesNode, bool toConfigureTitles = true);
	void maximizeWindow_IfNotOverridden(berry::IWorkbenchWindow::Pointer window);

protected:
	QScopedPointer<berry::IPartListener> viewPartListener;
};

#endif /*MAINWORKBENCHWINDOWADVISOR_H_*/
