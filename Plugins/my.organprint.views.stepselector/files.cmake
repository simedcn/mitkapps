set(CPP_FILES
  src/internal/my_awesomeproject_stepselector_PluginActivator.cpp
  src/internal/StepSelector.cpp
)

set(UI_FILES
  src/internal/StepSelectorControls.ui
)

set(MOC_H_FILES
  src/internal/my_awesomeproject_stepselector_PluginActivator.h
  src/internal/StepSelector.h
)

# List of resource files that can be used by the plugin system without loading
# the actual plugin. For example, the icon that is typically displayed in the
# plugin view menu at the top of the application window.
set(CACHED_RESOURCE_FILES
  resources/AwesomeIcon.png
  resources/Styles.qss
  plugin.xml
)

set(QRC_FILES
  resources/StepSelector.qrc
)
