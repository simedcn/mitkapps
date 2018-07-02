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

#include "my_organprint_minimalapplication_Activator.h"

#include "MinimalApplication.h"
#include "MinimalPerspective.h"
#include "ctkPluginContext.h"

ctkPluginContext* my_organprint_minimalapplication_Activator::m_Context = nullptr;

void my_organprint_minimalapplication_Activator::start(ctkPluginContext *context)
{
  BERRY_REGISTER_EXTENSION_CLASS(MinimalApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(MinimalPerspective, context)

  m_Context = context;
}

void my_organprint_minimalapplication_Activator::stop(ctkPluginContext *context)
{
  Q_UNUSED(context)
  m_Context = nullptr;
}

ctkPluginContext *my_organprint_minimalapplication_Activator::GetContext()
{
  return m_Context;
}
