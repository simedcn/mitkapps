#ifndef inova_popeproject_apps_mainapplication_Activator_H
#define inova_popeproject_apps_mainapplication_Activator_H

#include <ctkPluginActivator.h>

class inova_popeproject_apps_mainapplication_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "inova_popeproject_apps_mainapplication")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext *context) override;
  void stop(ctkPluginContext *context) override;

  static ctkPluginContext* GetContext();

private:
  static ctkPluginContext * m_Context;
};

#endif // inova_popeproject_apps_mainapplication_Activator_H
