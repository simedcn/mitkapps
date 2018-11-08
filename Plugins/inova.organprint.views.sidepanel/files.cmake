set(CPP_FILES
  src/internal/inova_organprint_views_sidepanel_PluginActivator.cpp
  src/internal/STLExportService.cpp
  #src/internal/OrganPrintView.cpp
  src/internal/ImportPanel.cpp
  src/internal/ExportPanel.cpp
  src/internal/TissuePanel.cpp
  src/internal/ExternalShapeSmoother.cpp
  src/ExternalProgramOpener.cpp
)

set(UI_FILES
  #src/internal/OrganPrintViewControls.ui
  src/internal/ImportPanelControls.ui
  src/internal/ExportPanelControls.ui
  src/internal/TissuePanelControls.ui
)

set(MOC_H_FILES
  src/internal/inova_organprint_views_sidepanel_PluginActivator.h
  #src/internal/OrganPrintView.h
  src/internal/ImportPanel.h
  src/internal/ExportPanel.h
  src/internal/TissuePanel.h
  src/ExternalProgramOpener.h
)

# List of resource files that can be used by the plugin system without loading
# the actual plugin. For example, the icon that is typically displayed in the
# plugin view menu at the top of the application window.
set(CACHED_RESOURCE_FILES
  resources/OrganPrintIcon.png
  plugin.xml
)

set(QRC_FILES
  resources/OrganPrintView.qrc
)
