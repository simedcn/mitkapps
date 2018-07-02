/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "my_popeproject_toolsplugin_PluginActivator.h"
#include "ToolsPlugin.h"
#include "ToolsPluginPreferencePage.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* my_popeproject_toolsplugin_PluginActivator::context;

void my_popeproject_toolsplugin_PluginActivator::start(ctkPluginContext* context)
{
	this->context = context;

	BERRY_REGISTER_EXTENSION_CLASS(ToolsPlugin, context)
	BERRY_REGISTER_EXTENSION_CLASS(ToolsPluginPreferencePage, context)
}

void my_popeproject_toolsplugin_PluginActivator::stop(ctkPluginContext*)
{
}

ctkPluginContext* my_popeproject_toolsplugin_PluginActivator::GetPluginContext()
{
	return context;
}