/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkAwesomeWorkbenchAdvisor.h"
#include "internal/mitkExampleAppPluginActivator.h"

#include <QmitkExtWorkbenchWindowAdvisor.h>

#include <mitkWorkbenchUtil.h>

const QString QmitkAwesomeWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID =
    "my.awesomeproject.AwesomePerspective";

void
QmitkAwesomeWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);

  ctkPluginContext* context = mitk::ExampleAppPluginActivator::GetDefault()->GetPluginContext();
  mitk::WorkbenchUtil::SetDepartmentLogoPreference(":/ExampleApp/MyLogo.png", context);
}

berry::WorkbenchWindowAdvisor*
QmitkAwesomeWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
    berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  // -------------------------------------------------------------------
  // Here you could pass your custom Workbench window advisor
  // -------------------------------------------------------------------
  QmitkExtWorkbenchWindowAdvisor* advisor = new
      QmitkExtWorkbenchWindowAdvisor(this, configurer);
  advisor->SetWindowIcon(":/ExampleApp/icon_research.xpm");
  return advisor;
}

QString QmitkAwesomeWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSPECTIVE_ID;
}
