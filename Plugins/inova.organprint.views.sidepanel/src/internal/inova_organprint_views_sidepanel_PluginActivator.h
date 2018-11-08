
#ifndef inova_organprint_views_sidepanel_PluginActivator_h
#define inova_organprint_views_sidepanel_PluginActivator_h

#include <ctkPluginActivator.h>

class inova_organprint_views_sidepanel_PluginActivator
    : public QObject,
      public ctkPluginActivator
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "inova_organprint_views_sidepanel")
    Q_INTERFACES(ctkPluginActivator)

public:
    void start(ctkPluginContext* context);
    void stop(ctkPluginContext* context);
    static ctkPluginContext* GetPluginContext();
private:
    static ctkPluginContext * context;
};

#endif
