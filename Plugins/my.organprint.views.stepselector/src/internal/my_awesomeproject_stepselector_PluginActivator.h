
#ifndef my_organprint_stepselector_PluginActivator_h
#define my_organprint_stepselector_PluginActivator_h

#include <ctkPluginActivator.h>

class my_awesomeproject_stepselector_PluginActivator
  : public QObject,
    public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "my_organprint_views_stepselector")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static ctkPluginContext* GetPluginContext();

private:
  static ctkPluginContext* context;
};

#endif
