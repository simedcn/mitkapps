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

#include "my_awesomeproject_exampleplugin_PluginActivator.h"
//#include "AwesomeView.h"
#include "ImportPanel.h"
#include "ExportPanel.h"
#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

void my_awesomeproject_exampleplugin_PluginActivator::start(ctkPluginContext* context)
{
    //BERRY_REGISTER_EXTENSION_CLASS(AwesomeView, context);
    BERRY_REGISTER_EXTENSION_CLASS(orgpnt::ImportPanel, context);
    BERRY_REGISTER_EXTENSION_CLASS(orgpnt::ExportPanel, context);
}

void my_awesomeproject_exampleplugin_PluginActivator::stop(ctkPluginContext*)
{
}
