/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "MinimalApplication.h"

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
#include "my_organprint_minimalapplication_Activator.h"
//#include <QmitkStatusBar.h>


const std::vector<QString> MinimalApplication::VIEW_IDS =
{
    "my.organprint.views.importpanel",
    "my.organprint.views.exportpanel",
    "org.mitk.views.segmentation",
    "my.pacs.views.dicomview"
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
        ctkPluginContext* context = my_organprint_minimalapplication_Activator::GetContext();
        mitk::WorkbenchUtil::SetDepartmentLogoPreference(":/images/logo.png", context);
        return new OrganPrintWorkbenchWindowAdvisor(configurer);
    }

    QString GetInitialWindowPerspectiveId() override {
        return DEFAULT_PERSPECTIVE_ID;
    }
};

const QString MinimalWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "my.organprint.minimalperspective";

MinimalApplication::MinimalApplication()
{
}

MinimalApplication::~MinimalApplication()
{
}

QVariant MinimalApplication::Start(berry::IApplicationContext * /*context*/)
{
    QScopedPointer<berry::Display> display(berry::PlatformUI::CreateDisplay());

    QScopedPointer<MinimalWorkbenchAdvisor> wbAdvisor(new MinimalWorkbenchAdvisor());
    int code = berry::PlatformUI::CreateAndRunWorkbench(display.data(), wbAdvisor.data());

    // exit the application with an appropriate return code
    return code == berry::PlatformUI::RETURN_RESTART ? EXIT_RESTART : EXIT_OK;
}

void MinimalApplication::Stop()
{
}
