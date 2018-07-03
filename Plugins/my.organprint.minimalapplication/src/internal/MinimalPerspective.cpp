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

#include "MinimalPerspective.h"

#include "MinimalApplication.h"

// Berry
#include "berryIViewLayout.h"
#include "berryIQtStyleManager.h"
#include "my_organprint_minimalapplication_Activator.h"

MinimalPerspective::MinimalPerspective()
{
}

void MinimalPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
    QString editorArea = layout->GetEditorArea();
    QString stepSelectorId = "my.organprint.views.stepselector";
    layout->AddStandaloneView(stepSelectorId, false, berry::IPageLayout::LEFT, 0.2f, editorArea);
    layout->AddStandaloneView("org.mitk.views.datamanager", false, berry::IPageLayout::BOTTOM, 0.3f, stepSelectorId);
    QString prev_id = editorArea;
    for (auto& viewId : MinimalApplication::VIEW_IDS)
    {
        layout->AddStandaloneView(viewId, false, berry::IPageLayout::RIGHT, 0.7f, prev_id);
        auto view = layout->GetViewLayout(viewId);
        prev_id = viewId;
    }

    ctkPluginContext* context = my_organprint_minimalapplication_Activator::GetContext();

    ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
    if (styleManagerRef)
    {
        auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
        //berry::IQtStyleManager::StyleList styles;
        //styleManager->GetStyles(styles);
        //styleManager->SetStyle(styles.last().fileName);
        styleManager->SetStyle(":/my.organprint.minimalapplication/inoflat2.qss");
    }
}
