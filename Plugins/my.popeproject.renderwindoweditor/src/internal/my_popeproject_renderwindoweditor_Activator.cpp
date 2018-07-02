/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "my_popeproject_renderwindoweditor_Activator.h"

#include "PopeRenderWindowEditor.h"
#include "PopePreferencePage.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

ctkPluginContext* my_popeproject_renderwindoweditor_Activator::context;


void my_popeproject_renderwindoweditor_Activator::start(ctkPluginContext* context)
{
	this->context = context;

	BERRY_REGISTER_EXTENSION_CLASS(PopeRenderWindowEditor, context)
	BERRY_REGISTER_EXTENSION_CLASS(PopePreferencePage, context)
}

void my_popeproject_renderwindoweditor_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

ctkPluginContext* my_popeproject_renderwindoweditor_Activator::GetPluginContext()
{
	return context;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(my_popeproject_renderwindoweditor, my_popeproject_renderwindoweditor_Activator)
#endif
