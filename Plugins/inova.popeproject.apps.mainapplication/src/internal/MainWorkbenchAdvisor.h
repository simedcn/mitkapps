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

#ifndef MAINWORKBENCHADVISOR_H_
#define MAINWORKBENCHADVISOR_H_

// Berry
#include <berryIApplication.h>

// Qt
#include <QObject>
#include <QScopedPointer>

// Berry
#include <berryPlatformUI.h>
#include <berryQtWorkbenchAdvisor.h>

class MainWorkbenchAdvisor : public berry::QtWorkbenchAdvisor
{
public:
	static const QString DEFAULT_PERSPECTIVE_ID;

	void Initialize(berry::IWorkbenchConfigurer::Pointer configurer) override;
	berry::WorkbenchWindowAdvisor *CreateWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer) override;
	void PostStartup() override;
	QString GetInitialWindowPerspectiveId() override;
};

#endif /*MAINWORKBENCHADVISOR_H_*/
