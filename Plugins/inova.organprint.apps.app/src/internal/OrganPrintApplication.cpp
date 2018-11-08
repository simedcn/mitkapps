
#include "OrganPrintApplication.h"

#include <berryPlatformUI.h>

#include "OrganPrintWorkbenchAdvisor.h"

#include <QDebug>

OrganPrintApplication::OrganPrintApplication()
{
}

OrganPrintApplication::OrganPrintApplication(const OrganPrintApplication& other)
: QObject(other.parent())
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QVariant OrganPrintApplication::Start(berry::IApplicationContext* /*context*/)
{
  berry::Display* display = berry::PlatformUI::CreateDisplay();

  int code = berry::PlatformUI::CreateAndRunWorkbench(display, new OrganPrintWorkbenchAdvisor());

  // exit the application with an appropriate return code
  return code == berry::PlatformUI::RETURN_RESTART
              ? EXIT_RESTART : EXIT_OK;
}

void OrganPrintApplication::Stop()
{

}
