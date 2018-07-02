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

#ifndef MAINPERSPECTIVE_H_
#define MAINPERSPECTIVE_H_

// Berry
#include <berryIPerspectiveFactory.h>

// Qt
#include <QObject>

using namespace std;


class MainPerspective : public QObject, public berry::IPerspectiveFactory
{
	Q_OBJECT
	Q_INTERFACES(berry::IPerspectiveFactory)

public:
	MainPerspective();

	void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;

};

#endif /* MAINPERSPECTIVE_H_ */
