
#include "OrganPrintWorkbenchAdvisor.h"
#include "internal/mitkOrganPrintAppPluginActivator.h"

#include <QmitkExtWorkbenchWindowAdvisor.h>

#include <mitkWorkbenchUtil.h>

const QString OrganPrintWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID =
    "inova.organprint.OrganPrintPerspective";

void
OrganPrintWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);

  ctkPluginContext* context = mitk::OrganPrintAppPluginActivator::GetDefault()->GetPluginContext();
  mitk::WorkbenchUtil::SetDepartmentLogoPreference(":/OrganPrintApp/InovaLogo.png", context);
}

berry::WorkbenchWindowAdvisor*
OrganPrintWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
    berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  // -------------------------------------------------------------------
  // Here you could pass your custom Workbench window advisor
  // -------------------------------------------------------------------
  QmitkExtWorkbenchWindowAdvisor* advisor = new
      QmitkExtWorkbenchWindowAdvisor(this, configurer);
  advisor->SetWindowIcon(":/OrganPrintApp/icon_research.xpm");
  return advisor;
}

QString OrganPrintWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSPECTIVE_ID;
}
