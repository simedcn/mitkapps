#include "inova_registration_views_manualregistration_Activator.h"

#include "ManualRegistration.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE


ctkPluginContext* inova_registration_views_manualregistration_Activator::m_Context = 0;

void inova_registration_views_manualregistration_Activator::start(ctkPluginContext* context)
{
	BERRY_REGISTER_EXTENSION_CLASS(ManualRegistration, context)

	m_Context = context;
}

void inova_registration_views_manualregistration_Activator::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)

	m_Context = 0;
}

ctkPluginContext* inova_registration_views_manualregistration_Activator::GetContext()
{
	return m_Context;
}
