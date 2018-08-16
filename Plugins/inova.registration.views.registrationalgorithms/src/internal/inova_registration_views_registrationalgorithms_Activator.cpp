#include "inova_registration_views_registrationalgorithms_Activator.h"

#include "RegistrationAlgorithms.h"


ctkPluginContext* inova_registration_views_registrationalgorithms_Activator::m_Context = 0;

void inova_registration_views_registrationalgorithms_Activator::start(ctkPluginContext* context)
{
	BERRY_REGISTER_EXTENSION_CLASS(RegistrationAlgorithms, context)

	m_Context = context;
}

void inova_registration_views_registrationalgorithms_Activator::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)

	m_Context = 0;
}

ctkPluginContext* inova_registration_views_registrationalgorithms_Activator::GetContext()
{
	return m_Context;
}
