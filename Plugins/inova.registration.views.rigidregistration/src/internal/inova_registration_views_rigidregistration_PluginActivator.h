
#ifndef inova_registration_views_rigidregistration_PluginActivator_h
#define inova_registration_views_rigidregistration_PluginActivator_h

#include <ctkPluginActivator.h>

class inova_registration_views_rigidregistration_PluginActivator
  : public QObject,
    public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "inova_registration_views_rigidregistration")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static ctkPluginContext* GetPluginContext();

private:
  static ctkPluginContext* context;
};

#endif
