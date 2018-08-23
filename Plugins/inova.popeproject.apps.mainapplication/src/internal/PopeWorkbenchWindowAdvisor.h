#ifndef POPEWORKBENCHWINDOWADVISOR_H_
#define POPEWORKBENCHWINDOWADVISOR_H_

#include "../../Classes/MainWorkbenchWindowAdvisor.h"

#include <QmitkExtWorkbenchWindowAdvisor.h>

#include <service/event/ctkEventAdmin.h>

#include <QString>

using namespace std;


class PopeWorkbenchWindowAdvisor : public MainWorkbenchWindowAdvisor
{
	Q_OBJECT
public:
	PopeWorkbenchWindowAdvisor(berry::WorkbenchAdvisor* wbAdvisor, berry::IWorkbenchWindowConfigurer::Pointer configurer, const initializer_list<PluginDescriptor>& plugins, const QString& preferencesNode, bool addToolbarLabels = false);
	~PopeWorkbenchWindowAdvisor();

protected:
	void setCTKSignals();
	shared_ptr<vector<QAction*>> createDataActions();
	QToolBar* createToolbar_AndAddDataActions(QMainWindow* mainWindow, shared_ptr<vector<QAction*>> dataActions);
	void addDataActionsToMenu(QMainWindow* mainWindow, shared_ptr<vector<QAction*>> dataActions);
	void addSettingsToToolbar(QMainWindow* mainWindow, QToolBar* toolBar = nullptr);
	void manageRegistrationPlugins(berry::IWorkbenchWindow::Pointer window);

	QString GetWorkDirectory() const;

public:
	void PostWindowCreate() override;

signals:
	void OpenDICOMdataset(const ctkDictionary&);
	void OpenDataFolder(const ctkDictionary&);

protected slots:
	void on_action_OpenDICOM_triggered();
	void on_action_OpenFolder_triggered();
	void on_action_PACS_triggered();
	void on_action_Settings_triggered();
public slots:
	void on_MainWindow_ShowPACS_triggered(const ctkEvent& event);

protected:
	bool addToolbarLabels = false;
	QScopedPointer<berry::IPartListener> pluginListener;
};

#endif /*POPEWORKBENCHWINDOWADVISOR_H_*/
