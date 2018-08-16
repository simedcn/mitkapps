#include "inova_registration_views_comparison_Activator.h"

#include "RegistrationComparison.h"


ctkPluginContext* inova_registration_views_comparison_Activator::m_Context = 0;

void inova_registration_views_comparison_Activator::start(ctkPluginContext* context)
{
	BERRY_REGISTER_EXTENSION_CLASS(RegistrationComparison, context)

	m_Context = context;
}

void inova_registration_views_comparison_Activator::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)

	m_Context = 0;
}

ctkPluginContext* inova_registration_views_comparison_Activator::GetContext()
{
	return m_Context;
}
