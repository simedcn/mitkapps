
#include "MainWorkbenchAdvisor.h"
#include "MainWorkbenchWindowAdvisor.h"

//#include "QmitkExtWorkbenchWindowAdvisor.h"

#include <QPoint>
//#include <QMainWindow>
//#include <QMenuBar>

#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>


const QString MainWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "my.popeproject.mainperspective";


void MainWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
	berry::QtWorkbenchAdvisor::Initialize(configurer);

	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	auto popePreferencesNode = prefService->GetSystemPreferences()->Node("/my.popeproject.editors.renderwindow"); 
	bool toRestore = popePreferencesNode->GetBool("save session", true);
	configurer->SetSaveAndRestore(toRestore); //!! set false and check how the app works
}

berry::WorkbenchWindowAdvisor* MainWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
	// Set an individual initial size
	configurer->SetInitialSize(QPoint(1440, 800));
	// Set an individual title
	configurer->SetTitle("POPE Application");
	// Enable or disable UI elements
	//configurer->SetShowPerspectiveBar(false);
	//configurer->SetShowMenuBar(true);
	//configurer->SetShowToolBar(true);
	//configurer->SetShowStatusLine(true);

	//QmitkExtWorkbenchWindowAdvisor* advisor = new QmitkExtWorkbenchWindowAdvisor(this, configurer, true);
	MainWorkbenchWindowAdvisor* advisor = new MainWorkbenchWindowAdvisor(this, configurer);

	/// Exclude the help perspective from org.blueberry.ui.qt.help from the normal perspective list.
	// The perspective gets a dedicated menu entry in the help menu
	QList<QString> excludePerspectives;
	excludePerspectives.push_back("org.blueberry.perspectives.help");
//	excludePerspectives.push_back("my.popeproject.dicomperspective");
	advisor->SetPerspectiveExcludeList(excludePerspectives);

	/// Exclude some views from the normal view list.
	QList<QString> excludeViews;
	excludeViews.push_back("org.mitk.views.modules");
	excludeViews.push_back("org.blueberry.ui.internal.introview");
	//excludeViews.push_back("org.mitk.views.properties");
	//excludeViews.push_back("org.mitk.views.datamanager");
//	excludeViews.push_back("org.mitk.views.xnat.treebrowser");
	//excludeViews.push_back("org.blueberry.ui.editors");
	//excludeViews.push_back("org.mitk.views.imagenavigator");
	excludeViews.push_back("org.mitk.views.mitksurfacematerialeditor");
	//excludeViews.push_back("org.mitk.editors.dicomeditor");
	//excludeViews.push_back("org.mitk.views.imagecropper");
	//excludeViews.push_back("org.mitk.views.measurement");
	excludeViews.push_back("org.mitk.views.imagestatistics");
	//excludeViews.push_back("org.mitk.views.segmentation");
	//excludeViews.push_back("org.mitk.views.deformableclippingplane");
	//excludeViews.push_back("org.mitk.views.segmentationutilities");
	excludeViews.push_back("org.blueberry.views.helpcontents");
	excludeViews.push_back("org.blueberry.views.helpindex");
	excludeViews.push_back("org.blueberry.views.helpsearch");
	excludeViews.push_back("org.mitk.views.aicpregistration");

	advisor->SetViewExcludeList(excludeViews);

	/// Costomization.
	advisor->ShowVersionInfo(false);
	advisor->ShowMitkVersionInfo(false);
	//advisor->SetWindowIcon(":/org.mitk.gui.qt.extapplication/icon.png");
	//berry::IWorkbenchWindow::Pointer window = advisor->GetWindowConfigurer()->GetWindow();

	////return new berry::WorkbenchWindowAdvisor(configurer);
	//return new QmitkExtWorkbenchWindowAdvisor(this, configurer);
	return advisor;
}

void MainWorkbenchAdvisor::PostStartup()
{
	// Set the position
	auto configurer = this->GetWorkbenchConfigurer();
	//auto configurer = GetWindowConfigurer();
//	auto window = configurer->GetWorkbench()->GetActiveWorkbenchWindow();
	auto window = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();
//	window->GetShell()->SetLocation(20, 20);
	  
	/*berry::IWorkbenchWindow::Pointer window = this->GetWorkbenchConfigurer()->GetWorkbench()->GetActiveWorkbenchWindow();
	QMainWindow* mainWindow = static_cast<QMainWindow*>(window->GetShell()->GetControl());
	QMenuBar* menuBar = mainWindow->menuBar();
	QMenu* fileMenu = menuBar->addMenu("File");*/
}

QString MainWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
	return DEFAULT_PERSPECTIVE_ID;
}
