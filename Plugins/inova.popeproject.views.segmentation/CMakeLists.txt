project(inova_popeproject_views_segmentation)

mitk_create_plugin(
  EXPORT_DIRECTIVE POPE_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgetsExt MitkSegmentation AppInteraction MitkIOExt
)
