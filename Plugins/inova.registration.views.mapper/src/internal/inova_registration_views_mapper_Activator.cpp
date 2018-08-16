#include "inova_registration_views_mapper_Activator.h"

#include "RegistrationMapper.h"


ctkPluginContext* inova_registration_views_mapper_Activator::m_Context = 0;

void inova_registration_views_mapper_Activator::start(ctkPluginContext* context)
{
	BERRY_REGISTER_EXTENSION_CLASS(RegistrationMapper, context)

	m_Context = context;
}

void inova_registration_views_mapper_Activator::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)

	m_Context = 0;
}

ctkPluginContext* inova_registration_views_mapper_Activator::GetContext()
{
	return m_Context;
}
