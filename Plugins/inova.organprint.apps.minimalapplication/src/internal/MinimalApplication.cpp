
#include "MinimalApplication.h"
#include "inova_organprint_apps_minimalapplication_Activator.h"

// Berry
#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>
#include <berryWorkbenchPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <QStatusBar>
#include <QPoint>
#include <QmitkProgressBar.h>
#include <QMainWindow>
#include <QLayout>
#include <mitkWorkbenchUtil.h>
//#include <QmitkStatusBar.h>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions>
//#include "vtk_glew.h"


const std::vector<QString> MinimalApplication::VIEW_IDS =
{
    "inova.organprint.views.importpanel",
    "inova.organprint.views.exportpanel",
    "org.mitk.views.segmentation",
    "inova.organprint.views.tissuepanel",
    "inova.pacs.views.dicomview"
};


class OrganPrintWorkbenchWindowAdvisor : public berry::WorkbenchWindowAdvisor
{
public:
    OrganPrintWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer)
        : berry::WorkbenchWindowAdvisor(configurer)
    {}

    void PostWindowCreate() override
    {

        berry::WorkbenchWindowAdvisor::PostWindowCreate();

        berry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
        QMainWindow* mainWindow = static_cast<QMainWindow*>(window->GetShell()->GetControl());
        if (window == nullptr)
            return;

        auto   qStatusBar = new QStatusBar();
        //auto  statusBar = new QmitkStatusBar(qStatusBar);
        auto  progBar = new QmitkProgressBar();
        //progBar->SetSizeGripEnabled(false);
        qStatusBar->addPermanentWidget(progBar, 100);
        qStatusBar->setSizeGripEnabled(false);
        //progBar->hide();
        progBar->Progress(100);
        mainWindow->setStatusBar(qStatusBar);
        mainWindow->centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);

        /*
            berry::IWorkbenchPage::Pointer page = window->GetActivePage();
            if (page == nullptr)
                return;

            /// Open the first view.
            for (unsigned long i = 0; i < MinimalApplication::VIEW_IDS.size(); i++)
            {
                const auto& view_id = MinimalApplication::VIEW_IDS[i];
                berry::IViewPart::Pointer view = page->FindView(view_id);
                bool to_open = (i == 0);
                if (view != nullptr && to_open)
                {   // Show view

                    view = page->ShowView(view_id);
                    //view->SetStyleSheets("backgroud-color:#ff0000");
                }
                else if (view != nullptr && !to_open)
                {   // Hide view
                    page->HideView(view);
                }
            }
            */

        /// Maximize the window.
        auto shell = window->GetShell();
        if (shell == nullptr)
            return;
        shell->SetMaximized(true);
    }
};


class MinimalWorkbenchAdvisor : public berry::QtWorkbenchAdvisor
{
public:
    static const QString DEFAULT_PERSPECTIVE_ID;

    berry::WorkbenchWindowAdvisor *CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer) override
    {
        // Set an individual initial size
        configurer->SetInitialSize(QPoint(1200, 800));
        // Set an individual title
        configurer->SetTitle("OrganPrint App");
        // Enable or disable the perspective bar
        configurer->SetShowPerspectiveBar(false);
        configurer->SetShowMenuBar(true);
        //return new berry::WorkbenchWindowAdvisor(configurer);
        ctkPluginContext* context = inova_organprint_apps_minimalapplication_Activator::GetContext();
        mitk::WorkbenchUtil::SetDepartmentLogoPreference("", context);
        mitk::WorkbenchUtil::SetDepartmentLogoPreference("", context);
        //mitk::WorkbenchUtil::SetDepartmentLogoPreference(":/images/organ-print_fullHD.png",context);
        //mitk::WorkbenchUtil::SetDepartmentLogoPreference(":/hello.png",context);
        return new OrganPrintWorkbenchWindowAdvisor(configurer);
    }

    QString GetInitialWindowPerspectiveId() override {
        return DEFAULT_PERSPECTIVE_ID;
    }
};

const QString MinimalWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "inova.organprint.views.minimalperspective";

MinimalApplication::MinimalApplication()
{
}

MinimalApplication::~MinimalApplication()
{
}

QVariant MinimalApplication::Start(berry::IApplicationContext * /*context*/)
{
	/// Initialize the app.
    QScopedPointer<berry::Display> display(berry::PlatformUI::CreateDisplay());

    QScopedPointer<MinimalWorkbenchAdvisor> wbAdvisor(new MinimalWorkbenchAdvisor());

	/// Check the OpenGL version.
	QOffscreenSurface surf;
	surf.create();
	QOpenGLContext ctx;
	ctx.create();
	ctx.makeCurrent(&surf);
	const char* gl_version = (const char*)ctx.functions()->glGetString(GL_VERSION);
	if (gl_version == nullptr)
	{
		QMessageBox::information(nullptr, tr("OpenGL Support"), tr("The system doesn't support OpenGL. The software is unlikely to function correctly. Please update or reinstall your graphics driver and/or hardware."));
	}
	else
	{
		std::string version = gl_version;
		int major_version = -1;
		int minor_version = -1;
		if (version.length() >= 3)
		{
			std::string vers = (version.length() > 3) ? version.substr(0, 3) : version;
			try
			{
				major_version = std::stoi(vers.substr(0, 1));
				minor_version = std::stoi(vers.substr(2, 1));
			}
			catch (...)
			{
				major_version = minor_version = -1;
			}
		}
		if (major_version < 3 || (major_version == 3 && minor_version < 2))
		{
			const char* gl_renderer = (const char*)ctx.functions()->glGetString(GL_RENDERER);
			std::string renderer = (gl_renderer == nullptr) ? "" : gl_renderer;
			std::stringstream ss;
			ss << "The system doesn't support OpenGL 3.2 or higher. The software is unlikely to function correctly. ";
			ss << "Please update or reinstall your graphics driver.\n\n";
			if (version.length() > 0)
				ss << "Version of OpenGL in the system: " << version << ".";
			if (renderer.length() > 0)
				ss << "\nGraphics: " << renderer << ".";
			QMessageBox::information(nullptr, tr("OpenGL 3.2 Support"), QString::fromStdString(ss.str()));
		}
	}
	ctx.doneCurrent();

	/// Start the app.
    int code = berry::PlatformUI::CreateAndRunWorkbench(display.data(), wbAdvisor.data());

    // exit the application with an appropriate return code
    return code == berry::PlatformUI::RETURN_RESTART ? EXIT_RESTART : EXIT_OK;
}

void MinimalApplication::Stop()
{
}
