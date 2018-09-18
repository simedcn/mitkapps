mitk_create_module(PopeLib
  DEPENDS PUBLIC MitkCore MitkQtWidgets MitkQtWidgetsExt MitkMatchPointRegistration
  PACKAGE_DEPENDS
    PUBLIC Qt5|Core GDCM CTK|CTKWidgets
    PRIVATE ITK|ITKIOImageBase+ITKIOGDCM DCMTK
)

add_subdirectory(cmdapps)