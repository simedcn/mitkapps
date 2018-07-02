/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 13820 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkBaseApplication.h>

#include <QVariant>

class MyBaseApplication : public mitk::BaseApplication
{
public:
	MyBaseApplication(int argc, char **argv);
	void defineOptions(Poco::Util::OptionSet &options) override;
};

int main(int argc, char** argv)
{
	MyBaseApplication myApp(argc, argv);
	myApp.setApplicationName("POPEApp");
	myApp.setOrganizationName("POPEOrg");

	/*// Preload the org.mitk.gui.qt.ext plug-in (and hence also QmitkExt) to speed
	// up a clean-cache start. This also works around bugs in older gcc and glibc implementations,
	// which have difficulties with multiple dynamic opening and closing of shared libraries with
	// many global static initializers. It also helps if dependent libraries have weird static
	// initialization methods and/or missing de-initialization code.
	QStringList preloadLibs;
	preloadLibs << "liborg_mitk_gui_qt_ext";
	myApp.setPreloadLibraries(preloadLibs);
	myApp.setProperty(mitk::BaseApplication::PROP_PRODUCT, "org.mitk.gui.qt.extapplication.workbench");*/

	// -------------------------------------------------------------------
	// Here you can switch to your customizable application:
	// -------------------------------------------------------------------
	myApp.setProperty(mitk::BaseApplication::PROP_APPLICATION, "my.popeproject.mainapplication");

	return myApp.run();
}

MyBaseApplication::MyBaseApplication(int argc, char **argv)
	: BaseApplication(argc, argv)
{}

void MyBaseApplication::defineOptions(Poco::Util::OptionSet &options)
{
	//auto window = berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow();
	mitk::BaseApplication::defineOptions(options);
}