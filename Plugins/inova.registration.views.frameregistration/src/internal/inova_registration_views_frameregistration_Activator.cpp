#include "inova_registration_views_frameregistration_Activator.h"

#include "FrameRegistration.h"


ctkPluginContext* inova_registration_views_frameregistration_Activator::m_Context = 0;

void inova_registration_views_frameregistration_Activator::start(ctkPluginContext* context)
{
	BERRY_REGISTER_EXTENSION_CLASS(FrameRegistration, context)

	m_Context = context;
}

void inova_registration_views_frameregistration_Activator::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)

	m_Context = 0;
}

ctkPluginContext* inova_registration_views_frameregistration_Activator::GetContext()
{
	return m_Context;
}
