project(inova_pacs_views_dicomview)

mitk_create_plugin(
  EXPORT_DIRECTIVE INOVA_PACS_VIEWS_DICOMVIEW_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src internal
  MODULE_DEPENDS MitkCore MitkQtWidgetsExt MitkDicomUI MitkDicomRT MitkDicomRTIO MitkRTUI MitkDICOMReader PopeLib
  PACKAGE_DEPENDS PRIVATE DCMTK
)

# Workaround //!!
#   For some reason, the folder created by CPack (the PACKAGE project) does not contain required libraries.
install(DIRECTORY "${BOOST_ROOT}/src/DCMTK-build/bin/Release/" DESTINATION "bin/bin")
#   For some reason, the folder created by CPack (the PACKAGE project) does not contain dcmjpeg.dll and dcmnet.dll.
#   Without these files the plugin cannot be loaded.
install(DIRECTORY "${BOOST_ROOT}/src/DCMTK-build/bin/Release/" DESTINATION "bin" FILES_MATCHING PATTERN "dcmjpeg*"  PATTERN "dcmnet*")
