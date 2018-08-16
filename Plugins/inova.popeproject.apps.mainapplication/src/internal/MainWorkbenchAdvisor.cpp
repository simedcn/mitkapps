#include "MainWorkbenchAdvisor.h"
#include "PopeWorkbenchWindowAdvisor.h"

//#include <QmitkExtWorkbenchWindowAdvisor.h>

#include <QPoint>
//#include <QMainWindow>
//#include <QMenuBar>

#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <initializer_list>

const QString MainWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "inova.popeproject.perspectives.mainperspective";

const initializer_list<PluginDescriptor> plugins =
{
//	[order]		[id]											[name]									[role]			[title] [position]		[open]
	{ "10",		"org.mitk.views.datamanager",					"Data Manager",							"main",			"yes",	"left",			"yes" },
	{ "01",		"inova.popeproject.views.tools",				"POPE View",							"main",			"no",	"right",		"yes" },
	{ "08",		"inova.registration.views.rigidregistration",	"Rigid Registration",					"main",			"no",	"right",		"no"  },
	{ "02",		"inova.registration.views.manualregistration",	"Manual Registration",					"registration",	"no",	"right",		"no"  },
	{ "03",		"inova.registration.views.registrationalgorithms","Registration Algorithms",			"registration",	"no",	"right",		"no"  },
	{ "04",		"inova.registration.views.frameregistration",	"Frame Registration",					"registration",	"no",	"right",		"no"  },
	{ "05",		"inova.registration.views.comparison",			"Registration Comparison",				"registration",	"no",	"right",		"no"  },
	{ "06",		"inova.registration.views.mapper",				"Registration Mapper",					"registration",	"no",	"right",		"no"  },
	{ "07",		"inova.registration.views.visualizer",			"Registration Visualizer",				"registration",	"no",	"right",		"no"  },
	{ "21",		"org.mitk.views.basicimageprocessing",			"Image Processing",						"secondary", 	"no",	"right",		"no"  },
	{ "23",		"inova.popeproject.views.segmentation",			"Segmentation Tools",					"secondary",	"no",	"right",		"no"  },
	{ "24",		"org.mitk.views.segmentation",					"Segmentation",							"secondary",	"no",	"right",		"no"  },
	{ "25",		"org.mitk.views.segmentationutilities",			"Segmentation Utilities",				"secondary",	"no",	"right",		"no"  },
	{ "26",		"org.mitk.views.deformableclippingplane",		"Clipping Plane",						"secondary",	"no",	"right",		"no"  },
	{ "17",		"org.blueberry.views.logview",					"Log",									"secondary",	"no",	"right",		"no"  },
	{ "15",		"org.mitk.views.imagestatistics",				"Statistics",							"secondary",	"no",	"right",		"no"  },
	{ "11",		"org.mitk.views.imagenavigator",				"Image Navigator",						"secondary",	"no",	"bottom_right",	"yes" },
	{ "14",		"org.mitk.views.measurement",					"Measurement",							"secondary",	"no",	"bottom_right",	"no"  },
	{ "16",		"org.mitk.views.imagecropper",					"Image Cropper",						"secondary",	"no",	"right",		"no"  },
	{ "12",		"org.mitk.views.properties",					"Properties",							"secondary",	"no",	"bottom_left",	"no"  },
	{ "13",		"org.mitk.views.volumevisualization",			"Volume Visualization",					"secondary",	"no",	"bottom_left",	"no"  },
	{ "22",		"org.mitk.gui.qt.dicominspector",				"DICOM Inspector",						"secondary",	"no",	"bottom_left",	"no"  },
	//{ "09",	"inova.pacs.views.dicomview",					"PACS Browser",							"pacs",			"yes",	"bottom", 		"no"  },
	{ "27",		"org.mitk.views.xnat.treebrowser",				"XNAT Browser",							"pacs",			"yes",	"bottom", 		"no"  },
	//{ "36",	"org.mitk.views.matchpoint.manipulator",		"MatchPoint Registration Manipulator",	"secondary",	"no",	"right", 		"no"  },
	{ "30",		"org.mitk.views.matchpoint.algorithm.browser",	"MatchPoint Algorithm Browser",			"secondary",	"no",	"right", 		"no"  },
	//{ "31",	"org.mitk.views.matchpoint.algorithm.control",	"MatchPoint Algorithm Control",			"secondary",	"no",	"right", 		"no"  },
	//{ "32",	"org.mitk.views.matchpoint.algorithm.framereg",	"MatchPoint Frame Correction",			"secondary",	"no",	"right", 		"no"  },
	//{ "33",	"org.mitk.views.matchpoint.mapper",				"MatchPoint Mapper",					"secondary",	"no",	"right", 		"no"  },
	{ "34",		"org.mitk.views.matchpoint.algorithm.batch",	"MatchPoint Registration Batch Processing","secondary",	"no",	"right", 		"no"  },
	//{ "35",	"org.mitk.views.matchpoint.evaluation.editor",	"MatchPoint Registration Evaluator",	"secondary",	"no",	"right", 		"no"  },
	//{ "37",	"org.mitk.views.matchpoint.visualizer",			"MatchPoint Registration Vizualizer",	"secondary",	"no",	"right", 		"no"  },
	//{ "38",	"org.mitk.views.aicpregistration",				"AICP-Registration",					"secondary",	"no",	"right", 		"no"  },
};


void MainWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
	berry::QtWorkbenchAdvisor::Initialize(configurer);

	berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
	auto popePreferencesNode = prefService->GetSystemPreferences()->Node("/inova.popeproject.editors.renderwindow"); 
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
	MainWorkbenchWindowAdvisor* advisor = new PopeWorkbenchWindowAdvisor(this, configurer, ::plugins, "/inova.popeproject.mainapplication", false);

	/// Exclude the help perspective from org.blueberry.ui.qt.help from the normal perspective list.
	// The perspective gets a dedicated menu entry in the help menu
	QList<QString> excludePerspectives;
	excludePerspectives.push_back("org.blueberry.perspectives.help");
//	excludePerspectives.push_back("inova.popeproject.dicomperspective");
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
//	excludeViews.push_back("org.mitk.views.imagestatistics");
	//excludeViews.push_back("org.mitk.views.segmentation");
	//excludeViews.push_back("org.mitk.views.deformableclippingplane");
	//excludeViews.push_back("org.mitk.views.segmentationutilities");
	excludeViews.push_back("org.blueberry.views.helpcontents");
	excludeViews.push_back("org.blueberry.views.helpindex");
	excludeViews.push_back("org.blueberry.views.helpsearch");
	excludeViews.push_back("org.mitk.views.aicpregistration");
	excludeViews.push_back("inova.pacs.views.dicomviewdialog");
	excludeViews.push_back("org.mitk.views.matchpoint.manipulator");
	excludeViews.push_back("org.mitk.views.matchpoint.algorithm.control");
	excludeViews.push_back("org.mitk.views.matchpoint.algorithm.framereg");
	excludeViews.push_back("org.mitk.views.matchpoint.mapper");
	excludeViews.push_back("org.mitk.views.matchpoint.evaluation.editor");
	excludeViews.push_back("org.mitk.views.matchpoint.visualizer");

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
