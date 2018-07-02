
#include "ui_MainWindow.h"

#include "MainWindow.h"
#include "my_popeproject_renderwindoweditor_Activator.h"
#include "DataManager.h"

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
	auto pluginContext = my_popeproject_renderwindoweditor_Activator::GetPluginContext();
	ctkDictionary propsForSlot;
	ctkServiceReference ref = pluginContext->getServiceReference<ctkEventAdmin>();
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "pope/representation/START3D"; //"pope/representation/*";
		eventAdmin->subscribeSlot(this, SLOT(On_ToolsPlugin_Representation3DHasToBeInitiated(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "pope/representation/ENABLED3D";
		eventAdmin->subscribeSlot(this, SLOT(On_ToolsPlugin_NodeHasManyImages(ctkEvent)), propsForSlot);
		propsForSlot[ctkEventConstants::EVENT_TOPIC] = "pope/representation/SETRANGE";
		eventAdmin->subscribeSlot(this, SLOT(On_ToolsPlugin_SetRange(ctkEvent)), propsForSlot);
	}
	/// Creating an Event Publisher.
	if (ref)
	{
		ctkEventAdmin* eventAdmin = pluginContext->getService<ctkEventAdmin>(ref);
		eventAdmin->publishSignal(this, SIGNAL(Representation3D_changed(const ctkDictionary&)), "pope/representation/VIEWCHANGED", Qt::DirectConnection);
	}
	/// Add UI elements.
	ui.Patient_display_layout->addWidget(multiWidget);

	/// Costomize UI elements.
	// ...
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
	emit this->EnableAutoRotation(flag);

	/// Hide/show crosshair
	mitk::DataNode *n;
	n = multiWidget->GetWidgetPlane1(); if (n) n->SetVisibility(!flag);
	n = multiWidget->GetWidgetPlane2(); if (n) n->SetVisibility(!flag);
	n = multiWidget->GetWidgetPlane3(); if (n) n->SetVisibility(!flag);

	ctkDictionary properties;
	properties["enable3D"] = flag;
	emit Representation3D_changed(properties);

	/// Update representation
	renderer->RequestUpdateAll();

	ui.pushButton_ViewAll->setText(flag ? "View all" : "View 3D");
}

//LOAD DATA FROM EXPLORER
void MainWindow::on_pushButton_OpenDICOM_clicked()
{
	QString imagePath = QFileDialog::getOpenFileName(this, tr("Open File"), DataManager::GetWorkDirectory());
	emit this->ImageSetHasToBeLoaded(imagePath);
}
void MainWindow::on_pushButton_OpenFolder_clicked()
{
	QString imagePath = QFileDialog::getExistingDirectory(this, tr("Open File"), DataManager::GetWorkDirectory());
	emit this->ImageFolderHasToBeLoaded(imagePath);
}

void MainWindow::on_pushButton_PACS_clicked()
{
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
	for (QAction* menu_action : actions)
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
void MainWindow::On_ToolsPlugin_NodeHasManyImages(const ctkEvent& event)
{
	bool has_many_images = event.getProperty("has_many_images").toBool();
	ui.pushButton_ViewAll->setEnabled(has_many_images);
}
void MainWindow::On_ToolsPlugin_SetRange(const ctkEvent& event)
{
	mitk::ScalarType min = event.getProperty("min").toDouble();
	mitk::ScalarType max = event.getProperty("max").toDouble();
	if (this->multiWidget == nullptr)
		return;
	auto levelWindowWidget = multiWidget->levelWindowWidget;
	if (levelWindowWidget == nullptr)
		return;
	auto manager = levelWindowWidget->GetManager();
	if (manager == nullptr)
		return;
	auto levelWindow = manager->GetLevelWindow();
	levelWindow.SetRangeMinMax(min, max);
	//itk::ModifiedEvent e;
	//manager->Update(e);
//	levelWindow.SetLevelWindow(levelWindow.GetLevel(), levelWindow.GetWindow());
//	manager->SetLevelWindow(levelWindow);
	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}