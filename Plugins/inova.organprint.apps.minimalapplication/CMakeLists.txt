project(inova_organprint_apps_minimalapplication)

mitk_create_plugin(
  EXPORT_DIRECTIVE organprint_apps_minimalapplication_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgets MitkQtWidgetsExt MitkAppUtil AppInteraction
)
