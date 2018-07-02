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
  layout->AddView("org.mitk.views.datamanager", berry::IPageLayout::LEFT, 0.3f, editorArea);

  layout->AddView("my.organprint.views.sidepanel", berry::IPageLayout::RIGHT, 0.3f, editorArea);

  layout->AddView("org.mitk.views.segmentation", berry::IPageLayout::BOTTOM, 0.5f, "my.organprint.views.sidepanel");
  layout->AddView("org.mitk.views.properties", berry::IPageLayout::BOTTOM, 0.7f, "org.mitk.views.datamanager");


  //layout->GetViewLayout("my.awesomeproject.editors.renderwindow");
  
//  editor->SetCloseable(false);
//  editor->SetMoveable(false);

//  berry::IViewLayout::Pointer awesomeview = layout->GetViewLayout("org.mitk.views.segmentation");
//  awesomeview->SetCloseable(false);
//  awesomeview->SetMoveable(false);

  layout->GetViewLayout("org.mitk.views.datamanager");

  layout->SetFixed(true);

  ctkPluginContext* context = my_organprint_minimalapplication_Activator::GetContext();

  ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
  if (styleManagerRef)
  {
    auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
    //berry::IQtStyleManager::StyleList styles;
    //styleManager->GetStyles(styles);
    //styleManager->SetStyle(styles.last().fileName);
    styleManager->SetStyle(":/org.blueberry.ui.qt/inoflat.qss");
  }
}
