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
