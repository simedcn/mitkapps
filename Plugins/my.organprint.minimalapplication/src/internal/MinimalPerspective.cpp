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

    float selectorSize = 0.4f;
    float panelSize = 0.3f;
    QString editorArea = layout->GetEditorArea();
    QString stepSelectorId = "my.organprint.views.stepselector";
    layout->SetEditorAreaVisible(true);
    layout->AddStandaloneView(stepSelectorId, false, berry::IPageLayout::LEFT, selectorSize, editorArea);
    //layout->AddStandaloneViewPlaceholder(stepSelectorId,berry::IPageLayout::LEFT, selectorSize, editorArea,false);
    layout->AddStandaloneView("org.mitk.views.datamanager", false, berry::IPageLayout::RIGHT, 0.6f, editorArea);
    layout->AddStandaloneView("org.mitk.views.properties", true, berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.views.datamanager");
    //layout->AddStandaloneView("my.organprint.views.importpanel",false,berry::IPageLayout::LEFT,panelSize,editorArea);
    layout->AddStandaloneView("org.mitk.views.statusbar",false,berry::IPageLayout::BOTTOM,0.1f,editorArea);
    //layout->AddStandaloneView("inova.pacs.views.dicomview",false,berry::IPageLayout::BOTTOM,0.1,editorArea);



    QString prev_id = stepSelectorId;
    for (auto& viewId : MinimalApplication::VIEW_IDS)
    {
        if(viewId == MinimalApplication::VIEW_IDS[0]) {
            layout->AddStandaloneView(viewId,false,berry::IPageLayout::RIGHT,panelSize,stepSelectorId);
            // layout->AddStandaloneViewPlaceholder(viewId,berry::IPageLayout::RIGHT, panelSize, stepSelectorId,false);
        }
        else {
            layout->AddStandaloneViewPlaceholder(viewId,berry::IPageLayout::RIGHT, panelSize, stepSelectorId,false);
        }
        //auto view = layout->GetViewLayout(viewId);
        //prev_id = viewId;

    }

    layout->AddStandaloneView(stepSelectorId, false, berry::IPageLayout::LEFT, selectorSize, editorArea);

    //layout->ShowView("my.organprint.views.importpanel");

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
