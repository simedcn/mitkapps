project(my_organprint_minimalapplication)

mitk_create_plugin(
  EXPORT_DIRECTIVE ORGANPRINT_MINIMALAPPLICATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgets MitkQtWidgetsExt MitkAppUtil AppInteraction
  )
