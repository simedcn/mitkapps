
#ifndef inova_organprint_apps_minimalapplication_Activator_H
#define inova_organprint_apps_minimalapplication_Activator_H

#include <ctkPluginActivator.h>

class inova_organprint_apps_minimalapplication_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "inova_organprint_apps_minimalapplication")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext *context) override;
  void stop(ctkPluginContext *context) override;

  static ctkPluginContext* GetContext();

private:
  static ctkPluginContext * m_Context;
};

#endif // inova_organprint_apps_minimalapplication_Activator_H
