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

#ifndef my_organprint_minimalapplication_Activator_H
#define my_organprint_minimalapplication_Activator_H

#include <ctkPluginActivator.h>

class my_organprint_minimalapplication_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "my_organprint_minimalapplication")
  Q_INTERFACES(ctkPluginActivator)

public:
  void start(ctkPluginContext *context) override;
  void stop(ctkPluginContext *context) override;

  static ctkPluginContext* GetContext();

private:
  static ctkPluginContext * m_Context;
};

#endif // my_awesomeproject_minimalapplication_Activator_H
