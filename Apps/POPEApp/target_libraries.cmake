
# A list of plug-in targets which should be automatically enabled
# (or be available in external projects) for this application.

set(target_libraries
  # Enable plug-ins from this project
  inova_popeproject_apps_mainapplication

  # Require external plug-ins
  org_mitk_gui_qt_datamanager
  # org_mitk_gui_qt_xnat
)
