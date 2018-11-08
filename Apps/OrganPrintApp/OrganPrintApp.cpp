
#include <mitkBaseApplication.h>

#include <QVariant>

int main(int argc, char** argv)
{
  mitk::BaseApplication myApp(argc, argv);
  myApp.setApplicationName("OrganPrintApp");
  myApp.setOrganizationName("Inova DE");

  // -------------------------------------------------------------------
  // Here you can switch to your customizable application:
  // -------------------------------------------------------------------

  //myApp.setProperty(mitk::BaseApplication::PROP_APPLICATION, "org.mitk.qt.extapplication");
  myApp.setProperty(mitk::BaseApplication::PROP_APPLICATION,"inova.organprint.apps.minimalapplication");
  return myApp.run();
}
