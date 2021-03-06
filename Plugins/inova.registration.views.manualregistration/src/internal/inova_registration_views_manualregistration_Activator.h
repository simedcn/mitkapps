#ifndef inova_registration_views_manualregistration_Activator_h
#define inova_registration_views_manualregistration_Activator_h

#include <ctkPluginActivator.h>

class inova_registration_views_manualregistration_Activator :
	public QObject, public ctkPluginActivator
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "inova_registration_views_manualregistration_Activator")
	Q_INTERFACES(ctkPluginActivator)

public:

	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static ctkPluginContext* GetContext();

private:

	static ctkPluginContext* m_Context;

}; // inova_registration_views_manualregistration_Activator

#endif // inova_registration_views_manualregistration_Activator_h
