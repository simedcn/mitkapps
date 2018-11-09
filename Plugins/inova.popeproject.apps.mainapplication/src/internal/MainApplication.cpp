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

#include "MainApplication.h"
#include "MainWorkbenchAdvisor.h"

#include <QPoint>
//#include <QMainWindow>
//#include <QMenuBar>
#include <QMessageBox>

#include <berryPlatformUI.h>

//#include "vtk_glew.h"
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions>


MainApplication::MainApplication()
{
}

MainApplication::~MainApplication()
{
}

QVariant MainApplication::Start(berry::IApplicationContext* /*context*/)
{
	/// Initialize the app.
	QScopedPointer<berry::Display> display(berry::PlatformUI::CreateDisplay());

	QScopedPointer<MainWorkbenchAdvisor> wbAdvisor(new MainWorkbenchAdvisor());

	/// Check the OpenGL version.
	QOffscreenSurface surf;
	surf.create();
	QOpenGLContext ctx;
	ctx.create();
	ctx.makeCurrent(&surf);
	const char* gl_version = (const char*)ctx.functions()->glGetString(GL_VERSION);
	if (gl_version == nullptr)
	{
		QMessageBox::information(nullptr, tr("OpenGL Support"), tr("The system doesn't support OpenGL. The software is unlikely to function correctly. Please update or reinstall your graphics driver and/or hardware."));
	}
	else
	{
		std::string version = gl_version;
		int major_version = -1;
		int minor_version = -1;
		if (version.length() >= 3)
		{
			std::string vers = (version.length() > 3) ? version.substr(0, 3) : version;
			try
			{
				major_version = std::stoi(vers.substr(0, 1));
				minor_version = std::stoi(vers.substr(2, 1));
			}
			catch (...)
			{
				major_version = minor_version = -1;
			}
		}
		if (major_version < 3 || (major_version == 3 && minor_version < 2))
		{
			const char* gl_renderer = (const char*)ctx.functions()->glGetString(GL_RENDERER);
			std::string renderer = (gl_renderer == nullptr) ? "" : gl_renderer;
			std::stringstream ss;
			ss << "The system doesn't support OpenGL 3.2 or higher. The software is unlikely to function correctly. ";
			ss << "Please update or reinstall your graphics driver.\n\n";
			if (version.length() > 0)
				ss << "Version of OpenGL in the system: " << version << ".";
			if (renderer.length() > 0)
				ss << "\nGraphics: " << renderer << ".";
			QMessageBox::information(nullptr, tr("OpenGL 3.2 Support"), QString::fromStdString(ss.str()));
		}
	}
	ctx.doneCurrent();

	/// Start the app.
	int code = berry::PlatformUI::CreateAndRunWorkbench(display.data(), wbAdvisor.data());

	// exit the application with an appropriate return code
	return code == berry::PlatformUI::RETURN_RESTART ? EXIT_RESTART : EXIT_OK;
}

void MainApplication::Stop()
{
}
