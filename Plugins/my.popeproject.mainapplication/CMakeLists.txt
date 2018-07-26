project(my_popeproject_mainapplication)

mitk_create_plugin(
  EXPORT_DIRECTIVE my_popeproject_mainapplication_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  DEPENDS my_pacs_views_dicomview
  MODULE_DEPENDS PRIVATE MitkQtWidgetsExt MitkSceneSerialization MitkAppUtil PopeLib
)
