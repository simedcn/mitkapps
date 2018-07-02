project(my_popeproject_toolsplugin)

mitk_create_plugin(
  EXPORT_DIRECTIVE POPE_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgetsExt PopeLib MitkSegmentation AppInteraction MitkIOExt MitkChart
)
