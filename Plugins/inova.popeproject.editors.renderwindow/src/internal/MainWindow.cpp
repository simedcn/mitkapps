#include "ui_MainWindow.h"

#include "MainWindow.h"
#include "inova_popeproject_editors_renderwindow_Activator.h"
#include "DataManager.h"

#include <QmitkPreferencesDialog.h>
#include <mitkImageAccessByItk.h>
#include <QmitkRenderWindow.h>
//#include <QmitkSliceWidget.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkPointSet.h>
#include <mitkPointSetDataInteractor.h>
#include <mitkImageAccessByItk.h>
#include <mitkRenderingManager.h>
#include <mitkIOUtil.h>
#include <mitkDataStorage.h>

#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMFileReader.h>
#include <mitkDICOMTagsOfInterestHelper.h>
#include <mitkDICOMProperty.h>
#include <mitkDICOMFilesHelper.h>
#include <mitkDICOMITKSeriesGDCMReader.h>

#include <itkGDCMImageIO.h>
#include <itksys/SystemTools.hxx>
#include <gdcmDirectory.h>

#include <QString>
#include <QMessageBox>
#include <QToolBar>

#include <service/event/ctkEventConstants.h>

#include <berryWindow.h>
#include <berryMenuManager.h>
#include <berryIEditorInput.h>
#include <berryFileEditorInput.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryPlatformUI.h>
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryPlatform.h>

#include <limits>


MainWindow::MainWindow(QmitkStdMultiWidget* multiwidget, QWidget* parent) :
	QWidget(parent),
	multiWidget(multiwidget),
	ui(*new Ui::mainwindow)
{
	ui.setupUi(this);

	/// Register Qmitk-dependent global instances.
	QmitkRegisterClasses();

	//multiWidget->setStyleSheet("background-color:black; color: #FFF; border:0px");
	// Tell the multiWidget which DataStorage to render

	/// CTK signals.
	auto pluginContext = inova_popeproject_editors_renderwindow_Activator::GetPluginContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "pope/representation/START3D"; //"pope/representation/*";
		eventAdmin->subscribeSlot(this, SLOT(On_ToolsPlugin_Representation3DHasToBeInitiated(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "pope/representation/CROSSHAIR";
		eventAdmin->subscribeSlot(this, SLOT(On_ToolsPlugin_UpdateCrosshair(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "pope/representation/ENABLED3D";
		eventAdmin->subscribeSlot(this, SLOT(On_ToolsPlugin_NodeHasManyImages(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "pope/representation/SETRANGE";
		eventAdmin->subscribeSlot(this, SLOT(On_ToolsPlugin_SetRange(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "data/SELECTEDNODE";
		eventAdmin->subscribeSlot(this, SLOT(On_ToolsPlugin_SelectedNodeChanged(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "data/GONNAADDNEWDATANODE";
		eventAdmin->subscribeSlot(this, SLOT(On_RegistrationPlugin_GonnaAddNewDataNode(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "data/NEWDATANODEADDED";
		eventAdmin->subscribeSlot(this, SLOT(On_RegistrationPlugin_NewDataNodeAdded(ctkEvent)), propsForSlot);
	}
	/// Creating an Event Publisher.
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		eventAdmin->publishSignal(this, SIGNAL(Representation3D_changed(const ctkDictionary&)), "pope/representation/VIEWCHANGED", Qt::DirectConnection);
		eventAdmin->publishSignal(this, SIGNAL(ShowPACS_triggered(const ctkDictionary&)), "pope/show/PACS", Qt::DirectConnection);
	}
	/// Add UI elements.
	ui.Patient_display_layout->addWidget(multiWidget);

	/// Costomize UI elements.
	// ...

	/// LevelWindow manager.
	levelWindowManager = nullptr;
	if (multiWidget != nullptr && multiWidget->levelWindowWidget != nullptr)
		levelWindowManager = multiWidget->levelWindowWidget->GetManager();
	this->is_LevelWindowObserver_registered = false;
	this->status_changesInLevelWindow = DontSaveAnyChanges;
	savedLevelWindow_min = numeric_limits<double>::max();
	savedLevelWindow_max = numeric_limits<double>::max();
}

MainWindow::~MainWindow()
{
	delete this->multiWidget;
	delete &ui;
}

QmitkStdMultiWidget *MainWindow::getStdMultiWidget()
{
	return this->multiWidget;
}

void MainWindow::RegisterLevelWindowObserver()
{
	if (is_LevelWindowObserver_registered)
		return;

	if (levelWindowManager == nullptr)
	{
		if (this->multiWidget == nullptr)
			return;
		auto levelWindowWidget = multiWidget->levelWindowWidget;
		if (levelWindowWidget == nullptr)
			return;
		levelWindowManager = levelWindowWidget->GetManager();
	}
	if (levelWindowManager == nullptr)
		return;

	/// LevelWindow observer.

	auto levelWindowProperty = levelWindowManager->GetLevelWindowProperty();
	if (levelWindowProperty == nullptr)
	{
		auto command = itk::ReceptorMemberCommand<MainWindow>::New();
		command->SetCallbackFunction(this, &MainWindow::on_levelWindow_modified);
		levelWindowManager->AddObserver(itk::ModifiedEvent(), command);
		this->is_LevelWindowObserver_registered = true;
	}
	else
	{// would be better as command is called twice - command2 only once, but should be registered in the right moment
		auto command2 = itk::ReceptorMemberCommand<MainWindow>::New();
		command2->SetCallbackFunction(this, &MainWindow::on_levelWindow_modified);
		levelWindowProperty->AddObserver(itk::ModifiedEvent(), command2);
	}
}

void MainWindow::enable3DRepresentation(bool flag)
{
	auto renderer = mitk::RenderingManager::GetInstance();

	if (flag)
	{
		/// Show only 3D window
		multiWidget->changeLayoutToBig3D();
	}
	else
	{
		/// Show all windows
		multiWidget->changeLayoutToDefault();
	}
	/// Start/stop rotation
	emit EnableAutoRotation(flag);

	/// Hide/show crosshair
	updateCrosshair(flag);

	ctkDictionary properties;
	properties["enable3D"] = flag;
	emit Representation3D_changed(properties);

	/// Update representation
	renderer->RequestUpdateAll();

	ui.pushButton_ViewAll->setText(flag ? "View all" : "View 3D");
}
void MainWindow::updateCrosshair(bool is_autorotation_enabled)
{
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	auto settingsNode = prefService->GetSystemPreferences()->Node("/inova.popeproject.views.tools");
	bool is_crosshair_visible = !is_autorotation_enabled && settingsNode->GetBool("show crosshair", false);
	mitk::DataNode* n;
	n = multiWidget->GetWidgetPlane1(); if (n) n->SetVisibility(is_crosshair_visible);
	n = multiWidget->GetWidgetPlane2(); if (n) n->SetVisibility(is_crosshair_visible);
	n = multiWidget->GetWidgetPlane3(); if (n) n->SetVisibility(is_crosshair_visible);
}

//LOAD DATA FROM EXPLORER
void MainWindow::on_pushButton_OpenDICOM_clicked()
{
	QString imagePath = QFileDialog::getOpenFileName(this, tr("Open DICOM dataset"), DataManager::GetWorkDirectory());
	if (imagePath.size() == 0)
		return;

	emit this->ImageSetHasToBeLoaded(imagePath);
}
void MainWindow::on_pushButton_OpenFolder_clicked()
{
	QString imagePath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), DataManager::GetWorkDirectory());
	if (imagePath.size() == 0)
		return;

	emit this->ImageFolderHasToBeLoaded(imagePath);
}

void MainWindow::on_pushButton_PACS_clicked()
{
	ctkDictionary properties;
	emit ShowPACS_triggered(properties);
	return;

	//!! Test code: Change the main window location
	/*auto window = iwindow.Cast<berry::Window>();
	shell->SetLocation(20, 20);
	shell->SetMaximized(true);
	//shell->*/

	//!! Test code: Get MenuBar
	/*QMainWindow* mainWindow = qobject_cast<QMainWindow*>(shell->GetControl());
	if (mainWindow)
	{
		auto menubar = mainWindow->menuBar();
		auto actions = menubar->actions();
		menubar->addAction("asd");
		menubar->addSeparator();
		bool f = menubar->isEnabled();
		f = menubar->isHidden();
		f = menubar->isVisible();
		//QMenuBar* menuBar = window->GetMenuBarManager()->CreateMenuBar(mainWindow);
		//mainWindow->setMenuBar(menuBar);
	}*/

	berry::IWorkbench* currentWorkbench = berry::PlatformUI::GetWorkbench();
	if (currentWorkbench)
	{
		berry::IWorkbenchWindow::Pointer currentWorkbenchWindow = currentWorkbench->GetActiveWorkbenchWindow();
		if (currentWorkbenchWindow)
		{
			//berry::IWorkbenchWindow::Pointer window = this->GetWorkbenchConfigurer()->GetWorkbench()->GetActiveWorkbenchWindow();
			berry::IEditorInput::Pointer editorInput2(new berry::FileEditorInput(QString()));
			//currentWorkbenchWindow->GetActivePage()->OpenEditor(editorInput2, "org.mitk.editors.dicomeditor");
			currentWorkbenchWindow->GetActivePage()->OpenEditor(editorInput2, "org.mitk.editors.xnat.browser");
		}
	}
}

void MainWindow::on_pushButton_Settings_clicked()
{
	QmitkPreferencesDialog preferencesDialog(QApplication::activeWindow());
	//preferencesDialog.SetSelectedPage("org.mitk.gui.qt.application.DicomViewPreferencePage");
	preferencesDialog.exec();
	return;

	//!! Test code: preferences
	/*// get the preferences service
	berry::IPreferencesService* preferencesService = berry::Platform::GetPreferencesService();
	// get a preferences NODE
	auto generalPreferences = preferencesService->GetSystemPreferences()->Node("/General/Main");
	// put some values in a preferences NODE
	generalPreferences->PutBool("a bool	variable", true);
	generalPreferences->Put("a string", "blaaaa");
	generalPreferences->PutInt("int", 3);
	// get some values from a preferences NODE
	// always deliver a default value in case the key does not exist
	bool boolVar = generalPreferences->GetBool("a bool variable", false);
	QString stringVar = generalPreferences->Get("a string", "");
	int intVar = generalPreferences->GetInt("int", 0);
	// force immediate save of preferences node
	generalPreferences->Flush();*/

	//!! Test code: Print views loaded
	auto workbench = berry::PlatformUI::GetWorkbench();
	if (workbench == nullptr)
		return;
	auto iwindow = workbench->GetActiveWorkbenchWindow();
	if (iwindow == nullptr)
		return;
	auto shell = iwindow->GetShell();
	if (shell == nullptr)
		return;
	// Get MenuBar
	QMainWindow* mainWindow = qobject_cast<QMainWindow*>(shell->GetControl());
	if (mainWindow == nullptr)
		return;
	auto menubar = mainWindow->menuBar();
	if (menubar == nullptr)
		return;
	// Find Menu->Edit
	QAction* preferences_action = nullptr;
	// ..or.. QAction* action = filemenu->findChild<QAction*>("Preferences"); // after: preferencesAction->setObjectName("Preferences");
	auto actions = menubar->actions();
	for(QAction* menu_action : actions)
	{
		auto submenu = menu_action->menu();
		if (submenu && menu_action->text() == "&Edit")
		{
			for (QAction* action : submenu->actions())
			{
				if (action->isSeparator() || action->menu())
					continue;
				if (action->text() == "&Preferences...")
				{
					preferences_action = action;
					break;
				}
			}
			break;
		}
	}
	if (preferences_action)
	{
		preferences_action->activate(QAction::Trigger);
	}

	return;

	//!! Test code: Print views loaded
	berry::IWorkbench* currentWorkbench = berry::PlatformUI::GetWorkbench();
	if (currentWorkbench)
	{
		berry::IWorkbenchWindow::Pointer currentWorkbenchWindow = currentWorkbench->GetActiveWorkbenchWindow();
		if (currentWorkbenchWindow)
		{
			auto view = currentWorkbenchWindow->GetActivePage()->FindView("org.mitk.views.basicimageprocessing");
			if (view != nullptr)
			{
				auto prop = view->GetPartProperties();
				for (auto p : prop)
				{
					MITK_INFO << p;
				}
			}
			auto views = currentWorkbenchWindow->GetActivePage()->GetViews();
			for (auto v : views)
			{
				MITK_INFO << v->GetPartName() << "_" << v->GetTitleToolTip();
				auto prop = v->GetPartProperties();
				for (auto p : prop)
				{
					MITK_INFO << p;
				}
			}
		}
	}
}

void MainWindow::on_pushButton_ViewAll_clicked()
{
	int layout = multiWidget->GetLayout();
	bool enable3D = (layout == QmitkStdMultiWidget::LAYOUT_DEFAULT);
	enable3DRepresentation(enable3D);
}

void MainWindow::On_ToolsPlugin_Representation3DHasToBeInitiated(const ctkEvent& event)
{
	//QString reportTitle = event.getProperty("title").toString();
	//QString reportPath = event.getProperty("path").toString();
	bool enable3D = event.getProperty("enable3D").toBool();
	enable3DRepresentation(enable3D);
}
void MainWindow::On_ToolsPlugin_UpdateCrosshair(const ctkEvent& event)
{
	//bool is_crosshair_visible = event.getProperty("showCrosshair").toBool();
	updateCrosshair();
}
void MainWindow::On_ToolsPlugin_NodeHasManyImages(const ctkEvent& event)
{
	bool has_many_images = event.getProperty("has_many_images").toBool();
	ui.pushButton_ViewAll->setEnabled(has_many_images);
}
void MainWindow::On_ToolsPlugin_SetRange(const ctkEvent& event)
{
	mitk::ScalarType min = event.getProperty("min").toDouble();
	mitk::ScalarType max = event.getProperty("max").toDouble();

	RegisterLevelWindowObserver();

	try
	{
		auto levelWindow = levelWindowManager->GetLevelWindow();
		mitk::ScalarType center = (min + max) / 2;
		mitk::ScalarType span = max - min;
		mitk::LevelWindow newlevelWindow(levelWindow);
		newlevelWindow.SetLevelWindow(center, span);
		this->status_changesInLevelWindow = SaveAllChanges;
		levelWindowManager->SetLevelWindow(newlevelWindow);
		//MITK_INFO << "should be: " << min << "," << max << " is: " << levelWindow.GetLowerWindowBound() << "," << levelWindow.GetUpperWindowBound();
		//levelWindow.SetLevelWindow(center, span);
		//new_levelWindow.SetRangeMinMax(min, max);
		//manager->SetLevelWindow(new_levelWindow);
		//manager->Update(itk::NoEvent());
	}
	catch (itk::ExceptionObject e)
	{
		MITK_WARN << "Failed to update Level/Window: " << e;
	}
	this->status_changesInLevelWindow = SaveUIChanges;
	//itk::ModifiedEvent e;
	//manager->Update(e);
//	levelWindow.SetLevelWindow(levelWindow.GetLevel(), levelWindow.GetWindow());
//	manager->SetLevelWindow(levelWindow);
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
void MainWindow::On_ToolsPlugin_SelectedNodeChanged(const ctkEvent& event)
{
	/// Save the new data node name.
	QString new_node_name = event.getProperty("selectedDataNodeName").toString();
	if (this->selectedNodeName == new_node_name)
		return;

	this->selectedNodeName = new_node_name;
	this->status_changesInLevelWindow = DontSaveAnyChanges;
	savedLevelWindow_min = numeric_limits<double>::max();
	savedLevelWindow_max = numeric_limits<double>::max();
	if (new_node_name.isEmpty())
		return;

	/// Update LevelWindow.
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	QString node_name = "/datanode.settings." + new_node_name;
	auto settingsNode = prefService->GetSystemPreferences()->Node(node_name);
	double not_found = numeric_limits<double>::max();
	double min = settingsNode->GetDouble("LevelWindow_min", not_found);
	double max = settingsNode->GetDouble("LevelWindow_max", not_found);
	if (min != not_found && max != not_found)
	{
		ctkDictionary properties;
		properties["min"] = min;
		properties["max"] = max;
		ctkEvent event("SetLevelWindowRange", properties);
		this->status_changesInLevelWindow = SaveAllChanges;
		On_ToolsPlugin_SetRange(event); // will call:   mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
	else
	{
		mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	}
	this->status_changesInLevelWindow = SaveUIChanges;

}
void MainWindow::On_RegistrationPlugin_GonnaAddNewDataNode(const ctkEvent& event)
{
	On_DataManager_GonnaAddNewDataNode();
}
void MainWindow::On_RegistrationPlugin_NewDataNodeAdded(const ctkEvent& event)
{
	On_DataManager_NewDataNodeAdded();
}

void MainWindow::on_levelWindow_modified(const itk::EventObject& event)
{
	if (this->selectedNodeName.isEmpty() || this->status_changesInLevelWindow == DontSaveAnyChanges)
		return;

	QString node_name = "/datanode.settings." + this->selectedNodeName;
	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	auto settingsNode = prefService->GetSystemPreferences()->Node(node_name);
	try
	{
		auto levelWindow = levelWindowManager->GetLevelWindow();
		//MITK_INFO << "levelWindow changed: " << levelWindow.GetLevel() << "," << levelWindow.GetUpperWindowBound();
		auto min = levelWindow.GetLowerWindowBound();
		auto max = levelWindow.GetUpperWindowBound();
		double center = (min + max) / 2;
		double span = (max - min);
		double saved_center = (savedLevelWindow_min + savedLevelWindow_max) / 2;
		double saved_span = (savedLevelWindow_max - savedLevelWindow_min);
		bool is_center_changed = (abs(center - saved_center) > 0.001);
		bool is_span_changed = (abs(span - saved_span) > 0.001);
		if (!is_center_changed || !is_span_changed || this->status_changesInLevelWindow != SaveUIChanges)
		{
			settingsNode->PutDouble("LevelWindow_min", min);
			settingsNode->PutDouble("LevelWindow_max", max);
			savedLevelWindow_min = min;
			savedLevelWindow_max = max;
		}
	}
	catch (itk::ExceptionObject e)
	{
		//MITK_WARN << "Failed to update Level/Window: " << e;
	}
}

void MainWindow::On_DataManager_GonnaAddNewDataNode()
{
	// Workaround to distinguish between user's and MITK's automatic changes in LevelWindow.
	// Here we prevent changes in LevelWindow.
	this->status_changesInLevelWindow = DontSaveAnyChanges;
}
void MainWindow::On_DataManager_NewDataNodeAdded()
{
	// Workaround to distinguish between user's and MITK's automatic changes in LevelWindow.
	// Here we make changes in LevelWindow possible.
	this->status_changesInLevelWindow = SaveUIChanges;
}