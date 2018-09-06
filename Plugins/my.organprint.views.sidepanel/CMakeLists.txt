project(my_organprint_views_sidepanel)

mitk_create_plugin(
  EXPORT_DIRECTIVE MY_ORGANPRINT_VIEWS_SIDEPANEL_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  DEPENDS my_pacs_views_dicomview
  MODULE_DEPENDS PUBLIC MitkQtWidgetsExt MitkSegmentation MitkIOExt AppInteraction OrganPrintLib PopeLib
)
