#include "inova_registration_views_visualizer_Activator.h"

#include "RegistrationVisualizer.h"


ctkPluginContext* inova_registration_views_visualizer_Activator::m_Context = 0;

void inova_registration_views_visualizer_Activator::start(
    ctkPluginContext* context)
{
	BERRY_REGISTER_EXTENSION_CLASS(RegistrationVisualizer, context)

	m_Context = context;
}

void inova_registration_views_visualizer_Activator::stop(ctkPluginContext* context)
{
	Q_UNUSED(context)

	m_Context = 0;
}

ctkPluginContext* inova_registration_views_visualizer_Activator::GetContext()
{
	return m_Context;
}
