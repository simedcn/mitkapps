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

#include <QPoint>



const std::vector<QString> MinimalApplication::VIEW_IDS =
{
	//"my.awesomeproject.views.awesomeview",
	//"org.mitk.views.basicimageprocessing",
	"org.mitk.views.segmentationutilities",
	"org.mitk.views.properties"
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
		if (window == nullptr)
			return;
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
			{// Show view
				view = page->ShowView(view_id);
			}
			else if (view != nullptr && !to_open)
			{// Hide view
				page->HideView(view);
			}
		}

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
    configurer->SetInitialSize(QPoint(600, 400));
    // Set an individual title
    configurer->SetTitle("Minimal Application");
    // Enable or disable the perspective bar
    configurer->SetShowPerspectiveBar(false);
    configurer->SetShowMenuBar(true);
    //return new berry::WorkbenchWindowAdvisor(configurer);
    return new OrganPrintWorkbenchWindowAdvisor(configurer);
  }

  QString GetInitialWindowPerspectiveId() override { return DEFAULT_PERSPECTIVE_ID; }
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
