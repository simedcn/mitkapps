project(my_organprint_views_stepselector)

mitk_create_plugin(
  EXPORT_DIRECTIVE AWESOME_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgetsExt OrganPrintLib MitkSegmentation AppInteraction MitkIOExt
)
