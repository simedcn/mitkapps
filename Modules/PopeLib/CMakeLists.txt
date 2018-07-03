mitk_create_module(PopeLib
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS
    PUBLIC Qt5|Core GDCM
    PRIVATE ITK|ITKIOImageBase+ITKIOGDCM DCMTK
)

add_subdirectory(cmdapps)