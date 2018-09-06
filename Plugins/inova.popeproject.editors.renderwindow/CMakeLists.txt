PROJECT(inova_popeproject_editors_renderwindow)

mitk_create_plugin(
  EXPORT_DIRECTIVE inova_popeproject_editors_renderwindow_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkDICOMReader MitkMatchPointRegistration AppInteraction PopeLib
  PACKAGE_DEPENDS
    PUBLIC Qt5|Xml GDCM
    PRIVATE ITK|ITKIOImageBase+ITKIOGDCM DCMTK
  SUBPROJECTS MITK-CoreUI
  )
