PROJECT(inova_organprint_apps_app)

mitk_create_plugin(
  EXPORT_DIRECTIVE ORGANPRINT_APPS_APP_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
