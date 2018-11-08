
#include "MinimalPerspective.h"

#include "MinimalApplication.h"

// Berry
#include "berryIViewLayout.h"
#include "berryIQtStyleManager.h"
#include "inova_organprint_apps_minimalapplication_Activator.h"

MinimalPerspective::MinimalPerspective()
{}

void MinimalPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
    float selectorSize = 0.4f;
    float panelSize = 0.3f;
    QString editorArea = layout->GetEditorArea();
    QString stepSelectorId = "inova.organprint.views.stepselector";

    layout->SetEditorAreaVisible(false);
    layout->SetFixed(true);
    layout->GetViewLayout(editorArea)->SetCloseable(false);

    layout->AddStandaloneView(stepSelectorId, false, berry::IPageLayout::LEFT, selectorSize, editorArea);
    layout->AddStandaloneView("org.mitk.views.datamanager", false, berry::IPageLayout::RIGHT, 0.6f, editorArea);
    layout->AddStandaloneView("org.mitk.views.properties", false, berry::IPageLayout::BOTTOM, 0.5f, "org.mitk.views.datamanager");
    layout->AddStandaloneView("org.mitk.views.statusbar", false, berry::IPageLayout::BOTTOM, 0.1f, editorArea);
    //layout->AddStandaloneView("inova.pacs.views.dicomview", false, berry::IPageLayout::BOTTOM, 0.1, editorArea);

    layout->SetEditorAreaVisible(false);

    QString prev_id = stepSelectorId;
    for (auto& viewId : MinimalApplication::VIEW_IDS)
    {
        if(viewId == MinimalApplication::VIEW_IDS[0]) {
            layout->AddStandaloneView(viewId, false, berry::IPageLayout::RIGHT, panelSize, stepSelectorId);
            // layout->AddStandaloneViewPlaceholder(viewId, berry::IPageLayout::RIGHT, panelSize, stepSelectorId,false);
        }
        else
		{
            layout->AddStandaloneViewPlaceholder(viewId,berry::IPageLayout::RIGHT, panelSize, stepSelectorId,false);
        }
        //auto view = layout->GetViewLayout(viewId);
        //prev_id = viewId;
    }

    layout->AddStandaloneView(stepSelectorId, false, berry::IPageLayout::LEFT, selectorSize, editorArea);

    //layout->ShowView("inova.organprint.views.importpanel");

    ctkPluginContext* context = inova_organprint_apps_minimalapplication_Activator::GetContext();

    ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
    if (styleManagerRef)
    {
        auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
        //berry::IQtStyleManager::StyleList styles;
        //styleManager->GetStyles(styles);
        //styleManager->SetStyle(styles.last().fileName);
        styleManager->AddStyle(":/inova.organprint.apps.minimalapplication/inoflat.qss", "Inoflat 2");
        styleManager->SetStyle(":/inova.organprint.apps.minimalapplication/inoflat.qss");
    }
    layout->SetFixed(true);
}
