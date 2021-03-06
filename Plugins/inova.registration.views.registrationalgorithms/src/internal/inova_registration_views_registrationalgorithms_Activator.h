#ifndef inova_registration_views_registrationalgorithms_Activator_h
#define inova_registration_views_registrationalgorithms_Activator_h

#include <ctkPluginActivator.h>

class inova_registration_views_registrationalgorithms_Activator :
    public QObject, public ctkPluginActivator
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "inova_registration_views_registrationalgorithms")
	Q_INTERFACES(ctkPluginActivator)

public:
	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

	static ctkPluginContext* GetContext();

private:
	static ctkPluginContext* m_Context;

}; // inova_registration_views_registrationalgorithms_Activator

#endif // inova_registration_views_registrationalgorithms_Activator_h
