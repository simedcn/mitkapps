
#ifndef my_pacs_views_dicomview_Activator_H
#define my_pacs_views_dicomview_Activator_H

#include <ctkPluginActivator.h>

#include <berryIWindowListener.h>

class ctkPluginContext;

class my_pacs_views_dicomview_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "my_pacs_views_dicomview")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext *context) override;
  void stop(ctkPluginContext *context) override;

  static ctkPluginContext *GetPluginContext();

private:
  static ctkPluginContext *PluginContext;

private:
  QScopedPointer<berry::IWindowListener> wndListener;
};

#endif // my_pacs_views_dicomview_Activator_H
