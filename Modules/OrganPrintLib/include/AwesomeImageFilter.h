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

#ifndef AwesomeImageFilter_h
#define AwesomeImageFilter_h

#include <mitkImageToImageFilter.h>

// The following header file is generated by CMake and thus it's located in
// the build directory. It provides an export macro for classes and functions
// that you want to be part of the public interface of your module.
#include <OrganPrintLibExports.h>

// While you are free to derive directly from ITK filter base classes,
// MITK filter base classes provide typed accessor methods for the inputs
// and outputs, which will save you and your clients lots of manual casting.
class OrganPrintLib_EXPORT AwesomeImageFilter final : public mitk::ImageToImageFilter
{
public:
  // All classes that derive from an ITK-based MITK class need at least the
  // following two macros. Make sure you don't declare the constructor public
  // to force clients of your class to follow the ITK convention for
  // instantiating classes via the static New() method.
  mitkClassMacro(AwesomeImageFilter, mitk::ImageToImageFilter)
  itkFactorylessNewMacro(Self)

  itkSetMacro(Offset, int)
  itkGetMacro(Offset, int)

private:
  AwesomeImageFilter();
  ~AwesomeImageFilter();

  void GenerateData() override;

  int m_Offset;
};

#endif
