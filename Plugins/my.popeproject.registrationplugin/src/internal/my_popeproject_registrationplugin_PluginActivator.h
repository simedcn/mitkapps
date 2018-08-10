
#ifndef my_popeproject_registrationplugin_PluginActivator_h
#define my_popeproject_registrationplugin_PluginActivator_h

#include <ctkPluginActivator.h>

class my_popeproject_registrationplugin_PluginActivator
  : public QObject,
    public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "my_popeproject_registrationplugin")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static ctkPluginContext* GetPluginContext();

private:
  static ctkPluginContext* context;
};

#endif
