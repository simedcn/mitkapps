/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef ORGANPRINTPERSPECTIVE_H_
#define ORGANPRINTPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

class OrganPrintPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)
  
public:

	OrganPrintPerspective();
	OrganPrintPerspective(const OrganPrintPerspective& other);
  
  void CreateInitialLayout(berry::IPageLayout::Pointer layout);

};

#endif /* ORGANPRINTPERSPECTIVE_H_ */
