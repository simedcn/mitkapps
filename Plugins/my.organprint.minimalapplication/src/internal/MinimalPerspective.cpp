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

    float selectorSize = 0.35f;
    float panelSize = 0.35f;
    QString editorArea = layout->GetEditorArea();
    QString stepSelectorId = "my.organprint.views.stepselector";
    layout->AddStandaloneView(stepSelectorId, false, berry::IPageLayout::LEFT, selectorSize, editorArea);
    layout->AddStandaloneView("org.mitk.views.datamanager", false, berry::IPageLayout::RIGHT, 0.6f, editorArea);
    layout->AddStandaloneView("org.mitk.views.properties", true, berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.views.datamanager");
    layout->AddStandaloneView("my.organprint.views.importpanel",false,berry::IPageLayout::RIGHT,panelSize,stepSelectorId);
    layout->AddStandaloneView("org.mitk.views.statusbar",false,berry::IPageLayout::BOTTOM,0.1,editorArea);
    layout->SetEditorAreaVisible(false);



    QString prev_id = stepSelectorId;
    for (auto& viewId : MinimalApplication::VIEW_IDS)
    {
        layout->AddStandaloneViewPlaceholder(viewId,berry::IPageLayout::RIGHT, panelSize, stepSelectorId,false);
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
        styleManager->AddStyle(":/my.organprint.minimalapplication/inoflat.qss","Inoflat 2");
        styleManager->SetStyle(":/my.organprint.minimalapplication/inoflat.qss");
    }
}
