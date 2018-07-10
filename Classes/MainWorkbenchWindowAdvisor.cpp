
#include "MainWorkbenchWindowAdvisor.h"
#include "PartListenerForPlugins.h"

#include <berryUIException.h>
#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIProduct.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryQtPreferences.h>

#include <mitkLogMacros.h>

#include <QToolButton>


void sort_by_name(ViewDescriptors view_descs)
{
	// Sort the elements by their names
	sort(view_descs->begin(), view_descs->end(),
		[](pViewInfo a, pViewInfo b)
		{
			return (a->Name() < b->Name());
		}
	);
}
void sort_by_order(ViewDescriptors view_descs)
{
	// Sort the elements by their order
	sort(view_descs->begin(), view_descs->end(),
		[](pViewInfo a, pViewInfo b)
		{
			return (a->Order() < b->Order());
		}
	);
}


MainWorkbenchWindowAdvisor::MainWorkbenchWindowAdvisor(berry::WorkbenchAdvisor* wbAdvisor, berry::IWorkbenchWindowConfigurer::Pointer configurer, const initializer_list<PluginDescriptor>& plugins, const QString& preferencesNode, bool addProjectLabel)
	: QmitkExtWorkbenchWindowAdvisor(wbAdvisor, configurer)
	, addProjectLabel(addProjectLabel)
{
	// Set plugins
	PluginDescriptors::set(plugins);

	// Get settings
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	auto systemPrefs = prefService->GetSystemPreferences();
	this->mainApplicationPreferencesNode = systemPrefs->Node(preferencesNode);
}
MainWorkbenchWindowAdvisor::~MainWorkbenchWindowAdvisor()
{}

void MainWorkbenchWindowAdvisor::setPluginTitle(const PluginDescriptor& plugin, berry::IWorkbenchPage::Pointer page)
{
	if (page == nullptr)
	{
		berry::IWorkbenchWindow::Pointer workbenchWindow = this->GetWindowConfigurer()->GetWindow();
		if (workbenchWindow == nullptr)
			return;
		page = workbenchWindow->GetActivePage();
		if (page == nullptr)
			return;
	}
	berry::IViewPart::Pointer view = page->FindView(plugin.id);
	if (view == nullptr)
		return;
	auto pt = view.Cast<berry::WorkbenchPart>();
	if (pt == nullptr)
		return;
	if (plugin.show_title)
		pt->SetPartName(plugin.name);
	else
		pt->SetPartName(" ");
	pt->SetTitleToolTip(plugin.name);
}
void MainWorkbenchWindowAdvisor::setPluginTitle(pViewInfo viewInfo, berry::IWorkbenchPage::Pointer page)
{
	if (viewInfo == nullptr)
		return;

	if (page == nullptr)
	{
		berry::IWorkbenchWindow::Pointer workbenchWindow = this->GetWindowConfigurer()->GetWindow();
		//auto site = this->GetSite();
		//if (site == nullptr)
		//	return;
		//auto workbenchWindow = site->GetWorkbenchWindow();
		if (workbenchWindow == nullptr)
			return;
		page = workbenchWindow->GetActivePage();
		if (page == nullptr)
			return;
	}
	auto viewId = viewInfo->Id();
	berry::IViewPart::Pointer view = page->FindView(viewId);
	viewInfo->ConfigureTitle(view);
}
ViewDescriptors MainWorkbenchWindowAdvisor::createViewDescriptors()
 {
	auto view_descs = make_shared<vector<pViewInfo>>();

	// Get all views registered
	berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
	const QList<berry::IViewDescriptor::Pointer> viewDescriptors = viewRegistry->GetViews();

	// Exclude several views
	bool to_skip = false;
	for (const auto& viewDescriptor : viewDescriptors)
	{
	auto view_id = viewDescriptor->GetId();

	// If viewExcludeList is set, it contains the id-strings of views, which should not appear as an menu-entry in the menu
	if (viewExcludeList.size() > 0)
	{
		for (int i = 0; i < viewExcludeList.size(); i++)
		{
			if (viewExcludeList.at(i) == view_id)
			{
				to_skip = true;
				break;
			}
		}
		if (to_skip)
		{
			to_skip = false;
			continue;
		}
	}
	if (view_id == "org.blueberry.ui.internal.introview")
		continue;
	//if (view_id == "org.mitk.views.imagenavigator")
	//	continue;
	if (view_id == "org.mitk.views.viewnavigatorview")
		continue;

	auto viewInfo = make_shared<ViewInfo>(viewDescriptor);
	view_descs->push_back(viewInfo);
	}

	return view_descs;
 }
void MainWorkbenchWindowAdvisor::replaceViewActions(berry::IWorkbenchWindow::Pointer window, ViewDescriptors viewDescriptors)
{
	/// Delete actions created by the base class QmitkExtWorkbenchWindowAdvisor.
	for (auto& viewAction : viewActions)
	{
		delete viewAction;
	}
	viewActions.clear();

	/// Create new actions.
	for (const auto& view_descriptor : *viewDescriptors)
	{
		QAction* viewAction;
		viewAction = new ShowViewAction(window, view_descriptor);
		viewActions.push_back(viewAction);
	}
}
void MainWorkbenchWindowAdvisor::createViewMenu()
{
	if (!showViewMenuItem)
		return;
	if (mainWindow == nullptr)
		return;

	QMenuBar* menuBar = mainWindow->menuBar();
	// Find the Window menu
	//auto all_submenus = menuBar->findChildren<QMenu*>();
	QMenu* windowMenu = nullptr;
	QMenu* viewMenu = nullptr;
	auto submenus = menuBar->findChildren<QMenu*>("", Qt::FindChildOption::FindDirectChildrenOnly);
	for (auto& submenu : submenus)
	{
		QString title = submenu->title();
		if (title == "Window" || title == "&Window")
		{
			windowMenu = submenu;
			break;
		}
		//MITK_INFO << submenu->objectName();
	}
	if (windowMenu != nullptr)
	{
		windowMenu->addSeparator();
		viewMenu = windowMenu->addMenu("Show &View");
		if (viewMenu != nullptr)
		{
			viewMenu->setObjectName("Show View");
			viewMenu->addActions(viewActions);
		}
	}
	if (windowMenu == nullptr || viewMenu == nullptr)
	{
		MITK_WARN << "Failed to populate the menu Window-->Show_View.";
	}
}
void MainWorkbenchWindowAdvisor::updateMainToolbar()
{
/*	/// Delete text labels of SaveProject, closeProject and openDicomEditor actions.
	if (fileSaveProjectAction != nullptr)
		fileSaveProjectAction->setText("");
	else
		MITK_WARN << "Failed to set the Save Project action";

	if (closeProjectAction != nullptr)
		closeProjectAction->setText("");
	else
		MITK_WARN << "Failed to set the Close Project action";

	if (openDicomEditorAction != nullptr)
		openDicomEditorAction->setText("");
	else
		MITK_WARN << "Failed to set the Open Dicom Editor action";
*/
	if (mainWindow == nullptr)
		return;

	/// Find the main toolbar.
	QList<QToolBar*> toolbars = mainWindow->findChildren<QToolBar*>();
	QToolBar* mainActionsToolBar = nullptr;
	for (auto& toolbar : toolbars)
	{
		if (toolbar->objectName() == "mainActionsToolBar")
		{
			mainActionsToolBar = toolbar;
			break;
		}
	}

	if (mainActionsToolBar != nullptr)
	{
		auto actions = mainActionsToolBar->actions();
		mainActionsToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		
		if (addProjectLabel)
		{
			///// Delete all actions and save them.
			//for (auto action : mainActionsToolBar->actions())
			//{
			//	mainActionsToolBar->removeAction(action);
			//}

			/// Add the label "Project:".
			auto labelButton = new QToolButton;
			labelButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
			labelButton->setText("Project:");
			labelButton->setStyleSheet("background: transparent; margin: 0; padding: 0;");
			if (false && actions.size() == 0)
				mainActionsToolBar->addWidget(labelButton);
			else
				mainActionsToolBar->insertWidget(actions.front(), labelButton);

			///// Add all the saved actions.
			//mainActionsToolBar->addActions(actions);
		}

		/// Find fileOpen action and delete its text.
		QAction* fileOpenAction = nullptr;
		for (auto& action : mainActionsToolBar->actions())
		{
			if (action->text() == "&Open File...")
			{
				fileOpenAction = action;
				break;
			}
		}
		if (fileOpenAction != nullptr)
		{
			fileOpenAction->setText("&Open Project...");
			fileOpenAction->setToolTip("Open a .mitk project file");
		}
		else
		{
			MITK_WARN << "Failed to set the File Open action";
		}
	}
	else
	{
		MITK_WARN << "Failed to set the main toolbar";
	}
}
void MainWorkbenchWindowAdvisor::createViewToolbar(berry::IWorkbenchWindow::Pointer window, ViewDescriptors viewDescriptors)
{
	if (!showViewToolbar)
		return;
	if (mainWindow == nullptr)
		return;
	if (viewDescriptors->size() == 0)
		return;

	/// Find View actions.
	QList<QAction*> actions;
	for (auto viewInfo : *viewDescriptors)
	{
		QAction* viewAction = new ShowViewAction(window, viewInfo);
		actions.push_back(viewAction);
	}

	/// Add View toolbar.
	this->viewToolbar = new QToolBar;
	mainWindow->addToolBar(viewToolbar);
	viewToolbar->setObjectName("ViewToolbar");

	/// Add the label "Tabs:".
	auto labelButton = new QToolButton;
	labelButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
	labelButton->setText("Tabs:");
	labelButton->setStyleSheet("background: transparent; margin: 0; padding: 0;");
	viewToolbar->addWidget(labelButton);

	/// Add View actions.
	viewToolbar->addActions(actions);
}
void MainWorkbenchWindowAdvisor::findSubMenus(QMenu** windowMenu, QMenu** editMenu)
{
	if (mainWindow == nullptr)
		return;

	QMenuBar* menuBar = mainWindow->menuBar();
	// Find Window and Edit menu
	*windowMenu = nullptr;
	*editMenu = nullptr;
	auto submenus = menuBar->findChildren<QMenu*>("", Qt::FindChildOption::FindDirectChildrenOnly);
	for (auto& submenu : submenus)
	{
		QString title = submenu->title();
		if (title == "Window" || title == "&Window")
		{
			*windowMenu = submenu;
			if (*editMenu != nullptr)
				break;
		}
		if (title == "Edit" || title == "&Edit")
		{
			*editMenu = submenu;
			if (*windowMenu != nullptr)
				break;
		}
	}
}
void MainWorkbenchWindowAdvisor::setMenuPreferencesItem(QMenu* windowMenu, QMenu* editMenu)
{
	if (windowMenu != nullptr && editMenu != nullptr)
	{
		QAction* preferences_action = nullptr;
		bool is_separator_removed = false;
		for (QAction* action : windowMenu->actions())
		{
			if (action->menu())
				continue;
			// Remove separator from Window menu
			if (action->isSeparator())
			{
				windowMenu->removeAction(action);
				if (preferences_action != nullptr)
					break;
				is_separator_removed = true;
				continue;
			}
			// Find the Preferences action
			if (action->text() == "&Preferences...")
			{
				preferences_action = action;
				if (is_separator_removed)
					break;
			}
		}
		// Move the menu item
		if (preferences_action != nullptr)
		{
			editMenu->addSeparator();
			windowMenu->removeAction(preferences_action);
			editMenu->addAction(preferences_action);
			//editMenu->addAction("&Preferences...", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onEditPreferences()), QKeySequence("CTRL+P"));
		}
		else
		{
			MITK_WARN << "Failed to find the Preferences menu item";
		}
	}
	else
	{
		MITK_WARN << "Failed to set the Preferences menu";
	}
}
void MainWorkbenchWindowAdvisor::removeRedoAndUndo(QMenu* editMenu)
{
	if (mainWindow == nullptr)
		return;

	/// Find undoAction and redoAction.
	if (undoAction == nullptr || redoAction == nullptr)
	{
		if (editMenu != nullptr)
		{
			//QAction* undoAction = nullptr;   --> use this->undoAction
			//QAction* redoAction = nullptr;   --> use this->redoAction
			for (auto& action : editMenu->actions())
			{
				if (undoAction == nullptr && action->text() == "&Undo")
				{
					undoAction = action;
					if (redoAction != nullptr)
						break;
				}
				else if (redoAction == nullptr && action->text() == "&Redo")
				{
					redoAction = action;
					if (undoAction != nullptr)
						break;
				}
			}
		}
		else
		{
			MITK_WARN << "Failed to set the edit menu";
		}
	}

	/// Delete from menu.
	if (undoAction != nullptr)
		editMenu->removeAction(undoAction);
	else
		MITK_WARN << "Failed to set the undo action";

	if (redoAction != nullptr)
		editMenu->removeAction(redoAction);
	else
		MITK_WARN << "Failed to set the redo action";

	/// Find the main toolbar.
	QList<QToolBar*> toolbars = mainWindow->findChildren<QToolBar *>();
	QToolBar* mainActionsToolBar = nullptr;
	for (auto& toolbar : toolbars)
	{
		if (toolbar->objectName() == "mainActionsToolBar")
		{
			mainActionsToolBar = toolbar;
			break;
		}
	}

	if (undoAction == nullptr || redoAction == nullptr)
	{
		/// Find undoAction and redoAction.
		if (mainActionsToolBar != nullptr)
		{
			//QAction* undoAction = nullptr;   --> use this->undoAction
			//QAction* redoAction = nullptr;   --> use this->redoAction
			for (auto& action : mainActionsToolBar->actions())
			{
				if (undoAction == nullptr && action->text() == "&Undo")
				{
					undoAction = action;
					if (redoAction != nullptr)
						break;
				}
				else if (redoAction == nullptr && action->text() == "&Redo")
				{
					redoAction = action;
					if (undoAction != nullptr)
						break;
				}
			}
		}
		else
		{
			MITK_WARN << "Failed to set the main toolbar";
		}
	}

	/// Delete from toolbar.
	if (undoAction != nullptr)
		mainActionsToolBar->removeAction(undoAction);
	else
		MITK_WARN << "Failed to set the undo action";

	if (redoAction != nullptr)
		mainActionsToolBar->removeAction(redoAction);
	else
		MITK_WARN << "Failed to set the redo action";
}
void MainWorkbenchWindowAdvisor::removeImageNavigatorToggle()
{
	if (mainWindow == nullptr)
		return;

	/// Delete from toolbar.
	QList<QToolBar*> toolbars = mainWindow->findChildren<QToolBar *>();
	QToolBar* mainActionsToolBar = nullptr;
	for (auto& toolbar : toolbars)
	{
		if (toolbar->objectName() == "mainActionsToolBar")
		{
			mainActionsToolBar = toolbar;
			break;
		}
	}
	if (mainActionsToolBar != nullptr)
	{
		QAction* imageNavigatorAction = nullptr;
		for (auto& action : mainActionsToolBar->actions())
		{
			if (action->text() == "&Image Navigator")
			{
				imageNavigatorAction = action;
				break;
			}
		}
		if (imageNavigatorAction != nullptr)
			mainActionsToolBar->removeAction(imageNavigatorAction);
		else
			MITK_WARN << "Failed to set Image Navigator Toggle";
	}
	else
	{
		MITK_WARN << "Failed to set the main toolbar";
	}
}
void MainWorkbenchWindowAdvisor::manageViews(berry::IWorkbenchPage::Pointer page, bool toConfigureTitles)
{
	/// Open/close plugins.
	if (page == nullptr)
		return;

	for (const auto& plugin : PluginDescriptors::get())
	{
		berry::IViewPart::Pointer view = page->FindView(plugin.id);
		bool to_open = mainApplicationPreferencesNode->GetBool(plugin.id, plugin.is_open);
		if (view == nullptr && to_open)
		{// Show view
			view = page->ShowView(plugin.id);
		}
		else if (view != nullptr && !to_open)
		{// Hide view
			//bool isViewVisible = page->IsPartVisible(view);
			//if (isViewVisible)
				page->HideView(view);
		}
		/// If specified (\c toConfigureTitles), change plugin titles and tooltips.
		if (toConfigureTitles && view != nullptr)
		{
			auto pt = view.Cast<berry::WorkbenchPart>();
			if (pt == nullptr)
				return;
			if (plugin.show_title)
				pt->SetPartName(plugin.name);
			else
				pt->SetPartName(" ");
			pt->SetTitleToolTip(plugin.name);
		}
	}
}
void MainWorkbenchWindowAdvisor::maximizeWindow(berry::IWorkbenchWindow::Pointer window)
{
	if (window == nullptr)
		return;

	auto shell = window->GetShell();
	if (shell == nullptr)
		return;

	/// Change the main window location.
	auto rect = shell->GetBounds();
	auto location = rect.bottomLeft();
	//shell->SetLocation(max(0, location.x()), max(0, location.y()));
	bool is_changed = false;
	if (rect.x() < 0)
	{
		auto width = rect.width();
		rect.setX(0);
		rect.setWidth(width);
		is_changed = true;
	}
	if (rect.y() < 0)
	{
		auto height = rect.height();
		rect.setY(0);
		rect.setHeight(height);
		is_changed = true;
	}
	if (is_changed)
		shell->SetBounds(rect);

	/// Maximize the window.
	shell->SetMaximized(true);
}

void MainWorkbenchWindowAdvisor::PostWindowRestore()
{
	this->is_restored = true;
}
void MainWorkbenchWindowAdvisor::PostWindowCreate()
{
	/// Initialize UI.
	// Save the flags
	bool show_viewMenu = this->showViewMenuItem;
	bool show_viewToolbar = this->showViewToolbar;
	// Reset the flags
	this->showViewMenuItem = false;
	this->showViewToolbar = false;
	// Activate menu, toolbar, etc.
	QmitkExtWorkbenchWindowAdvisor::PostWindowCreate();
	// Restore the flags
	this->showViewMenuItem = show_viewMenu;
	this->showViewToolbar = show_viewToolbar;

	/// Open or close views according to what is specified in Settings.
	berry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
	berry::IWorkbenchPage::Pointer page = window->GetActivePage();
	// berry::IWorkbench* currentWorkbench = berry::PlatformUI::GetWorkbench();
	// berry::IWorkbenchPage::Pointer page = window->GetActivePage();
	manageViews(page, false);

	/// Register listeners.
	// Register View Part listener
	viewPartListener.reset(new PartListenerForPlugins(mainApplicationPreferencesNode));
	window->GetPartService()->AddPartListener(viewPartListener.data());
	// Register Preferences listener
	berry::IBerryPreferences::Pointer prefs = mainApplicationPreferencesNode.Cast<berry::IBerryPreferences>();
	if (prefs != nullptr)
	{
		auto delegate = berry::MessageDelegate1<MainWorkbenchWindowAdvisor, const berry::IBerryPreferences*>(this, &MainWorkbenchWindowAdvisor::OnPreferencesChanged);
		prefs->OnChanged.AddListener(delegate);
	}

	/// Create view descriptors (actions).
	auto viewDescriptors = createViewDescriptors();
	sort_by_name(viewDescriptors);

	/// Rename plugin titles.
	// the method manageViews() can set only the titles of specified plugins
	for (auto viewInfo : *viewDescriptors)
	{
		setPluginTitle(viewInfo, page);
	}

	/// Replace view actions.
	replaceViewActions(window, viewDescriptors);

	/// Create View menu.
	// very bad hack...
	this->mainWindow = qobject_cast<QMainWindow*> (window->GetShell()->GetControl());
	createViewMenu();

	/// Update Main Toolbar.
	updateMainToolbar();

	/// Create View toolbar.
	sort_by_order(viewDescriptors);
	createViewToolbar(window, viewDescriptors);

	/// Move Preferences from Window menu to Edit menu.
	QMenu* windowMenu = nullptr;
	QMenu* editMenu = nullptr;
	findSubMenus(&windowMenu, &editMenu);
	setMenuPreferencesItem(windowMenu, editMenu);

	/// Remove the redo and undo actions.
	removeRedoAndUndo(editMenu);
	/// Remove Image Navigator toggle
	removeImageNavigatorToggle();

	/// If the window state is restored:
	if (!is_restored)
	{
		/// Maximize window and correct position if needed.
		maximizeWindow(window);
	}
	// Test code: Perspective icon
	//QAction* perspAction = mapPerspIdToAction[(*perspIt)->GetId()];
	//std::string name = perspAction->iconText().toStdString();
	//perspAction->setIcon(QIcon::fromTheme("go-home", QIcon(":/org_mitk_icons/icons/tango/scalable/actions/go-home.svg")));
}
bool MainWorkbenchWindowAdvisor::PreWindowShellClose()
{
	bool shall_be_closed = QmitkExtWorkbenchWindowAdvisor::PreWindowShellClose();

	if (shall_be_closed)
	{
		// Unregister View Part listener
		berry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
		window->GetPartService()->RemovePartListener(viewPartListener.data());
		viewPartListener.reset();
		// Unregister Preferences listener
		berry::IBerryPreferences::Pointer prefs = mainApplicationPreferencesNode.Cast<berry::IBerryPreferences>();
		if (prefs != nullptr)
		{
			auto delegate = berry::MessageDelegate1<MainWorkbenchWindowAdvisor, const berry::IBerryPreferences*>(this, &MainWorkbenchWindowAdvisor::OnPreferencesChanged);
			prefs->OnChanged.RemoveListener(delegate);
		}
	}

	return shall_be_closed;
}

QString MainWorkbenchWindowAdvisor::ComputeTitle()
{
	berry::IWorkbenchWindowConfigurer::Pointer configurer =
		GetWindowConfigurer();
	berry::IWorkbenchPage::Pointer currentPage =
		configurer->GetWindow()->GetActivePage();
	berry::IEditorPart::Pointer activeEditor;
	if (currentPage)
	{
		activeEditor = lastActiveEditor.Lock();
	}

	QString title;
	berry::IProduct::Pointer product = berry::Platform::GetProduct();
	if (product.IsNotNull())
	{
		title = product->GetName();
	}
	if (title.isEmpty())
	{
		// instead of the product name, we use a custom variable for now
		title = productName;
	}

	if (currentPage)
	{
		if (activeEditor)
		{
			lastEditorTitle = activeEditor->GetTitleToolTip();
			if (!lastEditorTitle.isEmpty())
				title = lastEditorTitle + " - " + title;
		}
		berry::IPerspectiveDescriptor::Pointer persp =
			currentPage->GetPerspective();
		QString label = "";
		if (persp)
		{
			label = persp->GetLabel();
		}
		berry::IAdaptable* input = currentPage->GetInput();
		if (input && input != wbAdvisor->GetDefaultPageInput())
		{
			label = currentPage->GetLabel();
		}
		if (!label.isEmpty() && false)
		{
			title = label + " - " + title;
		}
	}

	if (false)
		title += " (Not for use in diagnosis or treatment of patients)";

	return title;
}
void MainWorkbenchWindowAdvisor::OnPreferencesChanged(const berry::IBerryPreferences* preferences)
{
	//!! somehow this function is called a lot of times
	auto prefs = dynamic_cast<const berry::IPreferences*>(preferences);
	if (prefs == nullptr)
		return;

	berry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
	berry::IWorkbenchPage::Pointer page = window->GetActivePage();
	manageViews(page);
}

//void QmitkExtWorkbenchWindowAdvisorHack::onAbout()
//{
//	auto   aboutDialog = new QmitkAboutDialog(QApplication::activeWindow(), nullptr);
//	std::string str = aboutDialog->GetAboutText().toStdString();
//	aboutDialog->open();
//}
