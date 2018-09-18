project(inova_popeproject_views_tools)

mitk_create_plugin(
  EXPORT_DIRECTIVE POPE_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS
	PRIVATE MitkQtWidgetsExt PopeLib MitkSegmentation AppInteraction MitkIOExt MitkChart
  PACKAGE_DEPENDS
    PRIVATE ITK|ITKIOImageBase+ITKOptimizers+ITKOptimizersv4+ITKTransform+ITKImageGrid+ITKImageFunction
)

# Workaround //!!
#   For some reason, the folder created by CPack (the PACKAGE project) does not contain the MatchPoint algorithms.
#   Without these files the registration cannot work.
#   add the required extensions for Linux, MacOS, etc.: ... FILES_MATCHING PATTERN "mdra*.dll" PATTERN "mdra*.EXTENSION"
#install(DIRECTORY "${MITK_DIR}/../ep/src/MatchPoint-build/bin/Release/" DESTINATION "bin" FILES_MATCHING PATTERN "mdra*.dll")


# Workaround //!!
#   For some reason, the folder created by CPack (the PACKAGE project) does not contain all the libraries necessary to run the application.
#   Without these files the plugins cannot be loaded.
#   Note: For Linux and MacOS, "*.dll" has to be replaced accordingly.
#install(DIRECTORY "${Qt5_DIR}/../../../bin/" DESTINATION "bin/qt" FILES_MATCHING PATTERN "*.dll")
#install(DIRECTORY "${MITK_DIR}/bin/Release/" DESTINATION "bin/mitk" FILES_MATCHING PATTERN "*.dll")
#install(DIRECTORY "${MITK_DIR}/../ep/bin/" DESTINATION "bin/ep" FILES_MATCHING PATTERN "*.dll")
#install(DIRECTORY "${MITK_DIR}/../ep/src/CTK-build/CTK-build/bin/Release/" DESTINATION "bin/plugins" FILES_MATCHING PATTERN "*.dll")
#install(DIRECTORY "${MITK_DIR}/bin/plugins/Release/" DESTINATION "bin/plugins" FILES_MATCHING PATTERN "*.dll")
#install(DIRECTORY "bin" DESTINATION "bin/..")