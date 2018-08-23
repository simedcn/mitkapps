project(inova_registration_views_stepselector)

mitk_create_plugin(
  EXPORT_DIRECTIVE inova_registration_views_stepselector_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS
	PRIVATE MitkQtWidgetsExt PopeLib AppInteraction
)
