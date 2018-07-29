set(CPP_FILES
  src/internal/my_awesomeproject_exampleplugin_PluginActivator.cpp
  src/internal/STLExportService.cpp
  #src/internal/AwesomeView.cpp
  src/internal/ImportPanel.cpp
  src/internal/ExportPanel.cpp
  src/internal/TissuPanel.cpp
)

set(UI_FILES
  #src/internal/AwesomeViewControls.ui
  src/internal/ImportPanelControls.ui
  src/internal/ExportPanelControls.ui
  src/internal/TissuPanelControls.ui
)

set(MOC_H_FILES
  src/internal/my_awesomeproject_exampleplugin_PluginActivator.h
  #src/internal/AwesomeView.h
  src/internal/ImportPanel.h
  src/internal/ExportPanel.h
  src/internal/TissuPanel.h
)

# List of resource files that can be used by the plugin system without loading
# the actual plugin. For example, the icon that is typically displayed in the
# plugin view menu at the top of the application window.
set(CACHED_RESOURCE_FILES
  resources/AwesomeIcon.png
  plugin.xml
)

set(QRC_FILES
  resources/AwesomeView.qrc
)
