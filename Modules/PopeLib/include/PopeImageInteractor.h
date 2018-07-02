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

#ifndef PopeImageInteractor_h
#define PopeImageInteractor_h

#include <mitkDataInteractor.h>
#include <itkIndex.h>

#include <PopeLibExports.h>

// See PopeImageFilter.h for details on typical class declarations
// in MITK. The actual functionality of this class is commented in its
// implementation file.

class PopeLib_EXPORT PopeImageInteractor final : public mitk::DataInteractor
{
public:
  mitkClassMacro(PopeImageInteractor, DataInteractor)
  itkFactorylessNewMacro(Self)

private:
  PopeImageInteractor();
  ~PopeImageInteractor();

  void ConnectActionsAndFunctions() override;
  void DataNodeChanged() override;

  void Paint(mitk::StateMachineAction* action, mitk::InteractionEvent* event);

  itk::Index<3> m_LastPixelIndex;
};

#endif
