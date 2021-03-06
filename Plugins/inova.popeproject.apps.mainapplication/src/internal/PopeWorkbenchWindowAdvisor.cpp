#include "PopeWorkbenchWindowAdvisor.h"
#include "PluginListener.h"
#include "inova_popeproject_apps_mainapplication_Activator.h"
#include <DicomViewDialog.h>
#include "PopeAboutDialog.h"

#include <QmitkPreferencesDialog.h>
#include <mitkLogMacros.h>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryQtStyleManager.h>
#include <berryPlatformUI.h>

#include <service/event/ctkEventConstants.h>

#include <QFileDialog>
#include <QCoreApplication>
#include <QToolButton>
#include <QApplication>


PopeWorkbenchWindowAdvisor::PopeWorkbenchWindowAdvisor(berry::WorkbenchAdvisor* wbAdvisor, berry::IWorkbenchWindowConfigurer::Pointer configurer, const initializer_list<PluginDescriptor>& plugins, const QString& preferencesNode, bool addToolbarLabels)
	: MainWorkbenchWindowAdvisor(wbAdvisor, configurer, plugins, preferencesNode, addToolbarLabels)
	, addToolbarLabels(addToolbarLabels)
{}
PopeWorkbenchWindowAdvisor::~PopeWorkbenchWindowAdvisor()
{}

void PopeWorkbenchWindowAdvisor::setCTKSignals()
{
	/// CTK slots.
	auto pluginContext = inova_popeproject_apps_mainapplication_Activator::GetContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "pope/show/PACS";
		eventAdmin->subscribeSlot(this, SLOT(on_MainWindow_ShowPACS_triggered(ctkEvent)), propsForSlot);
	}
	/// CTK signals.
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		eventAdmin->publishSignal(this, SIGNAL(OpenDICOMdataset(const ctkDictionary&)), "data/OPENDICOMDATASET", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(OpenDataFolder(const ctkDictionary&)), "data/OPENFOLDER", Qt::DirectConnection);
	}
}
shared_ptr<vector<QAction*>> PopeWorkbenchWindowAdvisor::createDataActions()
{
	auto dataActions = make_shared<vector<QAction*>>();

	/// Create "Open DICOM dataset" action.
	QAction* openFolderAction = new QAction();
	auto basePath = QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/places/");
	openFolderAction->setIcon(berry::QtStyleManager::ThemeIcon(basePath + "folder.svg"));
	openFolderAction->setText("&Load Folder...");
	openFolderAction->setToolTip("Load all data from a folder");
	QObject::connect(openFolderAction, SIGNAL(triggered(bool)), this, SLOT(on_action_OpenFolder_triggered()));

	/// Create "Open folder" action.
	QAction* openDICOMDatasetAction = new QAction();
	//basePath = QStringLiteral(":/org.mitk.gui.qt.ext/");
	basePath = QStringLiteral(":/images/");
	openDICOMDatasetAction->setIcon(berry::QtStyleManager::ThemeIcon(basePath + "dicom.svg"));
	openDICOMDatasetAction->setText("&Open DICOM Dataset...");
	openDICOMDatasetAction->setToolTip("Open DICOM data set");
	QObject::connect(openDICOMDatasetAction, SIGNAL(triggered(bool)), this, SLOT(on_action_OpenDICOM_triggered()));

	/// Create "Open folder" action.
	QAction* openPACSAction = new QAction();
	basePath = QStringLiteral(":/PACS/"); //?? does not work
	openPACSAction->setIcon(berry::QtStyleManager::ThemeIcon(basePath + "CreateSurface.png"));
	openPACSAction->setText("&PACS...");
	openPACSAction->setToolTip("Picture archiving and communication system (PACS)");
	QObject::connect(openPACSAction, SIGNAL(triggered(bool)), this, SLOT(on_action_PACS_triggered()));

	/// Add the actions to the list.
	dataActions->push_back(openFolderAction);
	dataActions->push_back(openDICOMDatasetAction);
	dataActions->push_back(openPACSAction);
	return dataActions;
}
QToolBar* PopeWorkbenchWindowAdvisor::createToolbar_AndAddDataActions(QMainWindow* mainWindow, shared_ptr<vector<QAction*>> dataActions)
{
	QToolBar* dataActionsToolBar = nullptr;
	if (mainWindow == nullptr)
		return dataActionsToolBar;

	/// Create the Data toolbar.
	dataActionsToolBar = new QToolBar;
	mainWindow->addToolBar(dataActionsToolBar);
	dataActionsToolBar->setObjectName("dataActionsToolBar");
	dataActionsToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

	if (addToolbarLabels)
	{
		/// Add the label "Data:".
		auto labelButton = new QToolButton;
		labelButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
		labelButton->setText("Data:");
		labelButton->setStyleSheet("background: transparent; margin: 0; padding: 0;");
		dataActionsToolBar->addWidget(labelButton);
	}

	/// Add the actions to the Data toolbar.
	if (dataActionsToolBar != nullptr)
	{
		for (auto action : *dataActions)
		{
			dataActionsToolBar->addAction(action);
		}
	}

	return dataActionsToolBar;
}
void PopeWorkbenchWindowAdvisor::addDataActionsToMenu(QMainWindow* mainWindow, shared_ptr<vector<QAction*>> dataActions)
{
	/// Find the File menu.
	QMenuBar* menuBar = mainWindow->menuBar();
	if (menuBar == nullptr)
		return;
	QMenu* fileMenu = nullptr;
	auto submenus = menuBar->findChildren<QMenu*>("", Qt::FindChildOption::FindDirectChildrenOnly);
	if (submenus.size() == 0)
		return;
	for (auto& submenu : submenus)
	{
		QString title = submenu->title();
		QString objectName = submenu->objectName();
		//MITK_INFO << title << " " << objectName;
		if (title == "File" || title == "&File" || objectName == "FileMenu")
		{
			fileMenu = submenu;
			break;
		}
	}
	if (fileMenu == nullptr)
		fileMenu = submenus[1];// submenus.first();  //?? doesn't work for File menu, therefore using Edit menu instead
	if (fileMenu == nullptr)
		return;

	/// Add the actions to the menu.
	fileMenu->insertSeparator(fileMenu->actions().first());
	for (auto it = dataActions->rbegin(); it != dataActions->rend(); ++it)
	{
		const auto& action = *it;
		fileMenu->insertAction(fileMenu->actions().first(), action);
	}
}
void PopeWorkbenchWindowAdvisor::addSettingsToToolbar(QMainWindow* mainWindow, QToolBar* toolBar)
{
	/// If \c toolbar isn't specified, find the main toolbar.
	if (toolBar == nullptr)
	{
		QList<QToolBar*> toolbars = mainWindow->findChildren<QToolBar*>();
		for (auto& toolbar_i : toolbars)
		{
			if (toolbar_i->objectName() == "mainActionsToolBar")
			{
				toolBar = toolbar_i;
				break;
			}
		}
	}
	if (toolBar == nullptr)
		return;

	/// Create "Settings" action.
	QAction* settingsAction = new QAction();
	auto basePath = QStringLiteral(":/images/");
	QIcon icon(":/images/preferences-system.png");
	settingsAction->setIcon(icon); // berry::QtStyleManager::ThemeIcon(basePath + "preferences-system.png"));
	settingsAction->setText("&Settings...");
	settingsAction->setToolTip("Open Preferences");
	QObject::connect(settingsAction, SIGNAL(triggered(bool)), this, SLOT(on_action_Settings_triggered()));

	/// Add the action to the main toolbar.
	toolBar->addAction(settingsAction);
}
void PopeWorkbenchWindowAdvisor::manageRegistrationPlugins(berry::IWorkbenchWindow::Pointer workbenchWindow)
{
	if (workbenchWindow == nullptr)
		return;
	auto page = workbenchWindow->GetActivePage();
	if (page == nullptr)
		return;

	/// Close all registration subplugin, keeping only the stepselector plugin.
	const QString registration_plugins = "inova.registration.views.";
	const QString registration_stepselector = "inova.registration.views.stepselector";
	auto views = page->GetViewReferences();
	berry::IViewPart::Pointer stepselector;
	//bool is_subplugin = false;
	for (auto view : views)
	{
		if (view == nullptr)
			continue;
		QString view_id = view->GetId();
		if (view_id == registration_stepselector)
			continue;

		if (view_id.left(registration_plugins.length()) == registration_plugins)
		{
			try
			{
				page->HideView(view);
			}
			catch (const berry::PartInitException& e)
			{
				BERRY_ERROR << "Error: " << e.what();
			}
		}
	}
}

void PopeWorkbenchWindowAdvisor::PostWindowCreate()
{
	berry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
	QMainWindow* mainWindow = qobject_cast<QMainWindow*> (window->GetShell()->GetControl());

	/// Add Data toolbar before other initializations.
	setCTKSignals();
	auto dataActions = createDataActions();
	auto dataActionsToolBar = createToolbar_AndAddDataActions(mainWindow, dataActions);

	/// Initializations.
	MainWorkbenchWindowAdvisor::PostWindowCreate();

	if (window == nullptr)
		return;
	//berry::IWorkbenchPage::Pointer page = window->GetActivePage();
	//if (page == nullptr)
	//	return;
	if (mainWindow == nullptr)
		return;

	/// Manage registration subplugins.
	manageRegistrationPlugins(window);

	/// Register Plugin Visibility Listener.
	pluginListener.reset(new PluginListener());
	window->GetPartService()->AddPartListener(pluginListener.data());

	/// Add Data actions to menu.
	addDataActionsToMenu(mainWindow, dataActions);

	/// Add Settings to the main toolbar
	addSettingsToToolbar(mainWindow, dataActionsToolBar);
}

QString PopeWorkbenchWindowAdvisor::GetWorkDirectory() const
{
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	auto preferencesNode = prefService->GetSystemPreferences()->Node("/inova.popeproject.editors.renderwindow");
	QString def_path = QCoreApplication::applicationDirPath();
	QString path = preferencesNode->Get("data folder", def_path);
	return path;
}

void PopeWorkbenchWindowAdvisor::on_action_OpenDICOM_triggered()
{
	QString imagePath = QFileDialog::getOpenFileName(nullptr, tr("Open DICOM dataset"), GetWorkDirectory());
	if (imagePath.size() == 0)
		return;

	ctkDictionary properties;
	properties["imagePath"] = imagePath;
	emit OpenDICOMdataset(properties);
}
void PopeWorkbenchWindowAdvisor::on_action_OpenFolder_triggered()
{
	QString imagePath = QFileDialog::getExistingDirectory(nullptr, tr("Open Directory"), GetWorkDirectory());
	if (imagePath.size() == 0)
		return;

	ctkDictionary properties;
	properties["imagePath"] = imagePath;
	emit OpenDataFolder(properties);
}
void PopeWorkbenchWindowAdvisor::on_action_PACS_triggered()
{
	DicomViewDialog dialog(nullptr);
	dialog.exec();
	return;

	if (viewToolbar == nullptr)
		return;

	// !Test code

	// Find the PACS actions
	ShowViewAction* dicomviewAction = nullptr;
	ShowViewAction* xnatAction = nullptr;
	auto actions = viewToolbar->actions();
	for (auto action : actions)
	{
		auto showViewAction = dynamic_cast<ShowViewAction*>(action);
		if (!showViewAction)
			continue;
		if (showViewAction->Id() == "inova.pacs.views.dicomview")
		{
			dicomviewAction = showViewAction;
			if (xnatAction != nullptr)
				break;
		}
		else if (showViewAction->Id() == "org.mitk.views.xnat.treebrowser")
		{
			xnatAction = showViewAction;
			if (dicomviewAction != nullptr)
				break;
		}
	}
	// Show views
	if (xnatAction != nullptr)
		xnatAction->ShowView();
	if (dicomviewAction != nullptr)
		dicomviewAction->ShowView(); // give it focus

									 // If there are no actions, show preferences
	if (dicomviewAction == nullptr && xnatAction == nullptr)
	{
		QmitkPreferencesDialog preferencesDialog(QApplication::activeWindow());
		preferencesDialog.SetSelectedPage("org.mitk.gui.qt.application.DicomViewPreferencePage");
		preferencesDialog.exec();
	}
}
void PopeWorkbenchWindowAdvisor::on_action_Settings_triggered()
{
	QmitkPreferencesDialog preferencesDialog(QApplication::activeWindow());
	preferencesDialog.exec();
}
void PopeWorkbenchWindowAdvisor::on_MainWindow_ShowPACS_triggered(const ctkEvent& /*event*/)
{
	on_action_PACS_triggered();
}
void PopeWorkbenchWindowAdvisor::onAbout()
{
	auto   aboutDialog = new PopeAboutDialog(QApplication::activeWindow(), nullptr);
	aboutDialog->open();
}