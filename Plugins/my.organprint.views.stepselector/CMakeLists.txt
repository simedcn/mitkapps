project(my_organprint_views_stepselector)

mitk_create_plugin(
  EXPORT_DIRECTIVE AWESOME_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgetsExt OrganPrintLib MitkSegmentation AppInteraction MitkIOExt
)

# Workaround //!!
#   For some reason, the folder created by CPack (the PACKAGE project) does not contain all the libraries necessary to run the application.
#   Without these files the plugins cannot be loaded.
#   ___________________________________________________________
#
#   For Linux and MacOS, "*.dll" has to be replaced accordingly.
#

install(DIRECTORY "${Qt5_DIR}/../../../bin/" DESTINATION "bin/qt" FILES_MATCHING PATTERN "*.dll")
install(DIRECTORY "${MITK_DIR}/bin/Release/" DESTINATION "bin/mitk" FILES_MATCHING PATTERN "*.dll")
install(DIRECTORY "${MITK_DIR}/../ep/bin/" DESTINATION "bin/ep" FILES_MATCHING PATTERN "*.dll")
install(DIRECTORY "${MITK_DIR}/../ep/src/CTK-build/CTK-build/bin/Release/" DESTINATION "bin/plugins" FILES_MATCHING PATTERN "*.dll")
install(DIRECTORY "${MITK_DIR}/bin/plugins/Release/" DESTINATION "bin/plugins" FILES_MATCHING PATTERN "*.dll")
install(DIRECTORY "bin" DESTINATION "bin/..")
