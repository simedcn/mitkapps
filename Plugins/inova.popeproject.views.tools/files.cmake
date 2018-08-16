set(CPP_FILES
  src/internal/inova_popeproject_views_tools_PluginActivator.cpp
  src/internal/ToolsPlugin.cpp
  src/internal/TagTree.h
  src/internal/TagTree.cpp
  src/internal/ToolsPluginPreferencePage.cpp
  src/internal/ImageStatisticsCalculationThread.cpp
)

set(UI_FILES
  src/internal/ToolsPluginControls.ui
)

set(MOC_H_FILES
  src/internal/inova_popeproject_views_tools_PluginActivator.h
  src/internal/ToolsPlugin.h
  src/internal/ToolsPluginPreferencePage.h
  src/internal/ImageStatisticsCalculationThread.h
)

# List of resource files that can be used by the plugin system without loading
# the actual plugin. For example, the icon that is typically displayed in the
# plugin view menu at the top of the application window.
set(CACHED_RESOURCE_FILES
  resources/PopeIcon.png
  plugin.xml
)

set(QRC_FILES
  resources/ToolsPlugin.qrc
)
