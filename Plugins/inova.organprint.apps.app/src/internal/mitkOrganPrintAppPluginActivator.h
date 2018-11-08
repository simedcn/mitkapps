
#ifndef MITK_OrganPrintAPP_PLUGIN_ACTIVATOR_H_
#define MITK_OrganPrintAPP_PLUGIN_ACTIVATOR_H_

#include <berryAbstractUICTKPlugin.h>

#include <QString>

namespace mitk {

class OrganPrintAppPluginActivator : public berry::AbstractUICTKPlugin
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "inova_organprint_apps_app")
#endif
  Q_INTERFACES(ctkPluginActivator)
  
public:

  OrganPrintAppPluginActivator();
  ~OrganPrintAppPluginActivator();

  static OrganPrintAppPluginActivator* GetDefault();

  ctkPluginContext* GetPluginContext() const;

  void start(ctkPluginContext*);

  QString GetQtHelpCollectionFile() const;

private:

  static OrganPrintAppPluginActivator* inst;

  ctkPluginContext* context;

  mutable QString helpCollectionFile;
};

}

#endif /* MITK_OrganPrintAPP_PLUGIN_ACTIVATOR_H_ */
