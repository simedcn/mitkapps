
#ifndef inova_organprint_editors_renderwindoweditor_Activator_H_
#define inova_organprint_editors_renderwindoweditor_Activator_H_

#include <ctkPluginActivator.h>

/**
 * \ingroup org_mitk_gui_qt_stdmultiwidgeteditor
 */
class inova_organprint_editors_renderwindoweditor_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "inova_organprint_editors_renderwindoweditor")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);
  static ctkPluginContext* GetPluginContext();

private:
	static ctkPluginContext* context;

};

#endif /* inova_organprint_editors_renderwindoweditor_Activator_H_ */
