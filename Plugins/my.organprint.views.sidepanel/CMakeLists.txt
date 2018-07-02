project(my_organprint_sidepanel)

mitk_create_plugin(
  EXPORT_DIRECTIVE ORGANPRINT_SIDEPANEL_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgetsExt OrganPrintLib MitkSegmentation MitkIOExt AppInteraction
)
