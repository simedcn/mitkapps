set(CPP_FILES
  src/internal/inova_registration_views_stepselector_PluginActivator.cpp
  src/internal/RegistrationStepSelector.cpp
)

set(UI_FILES
  src/internal/RegistrationStepSelectorControls.ui
)

set(MOC_H_FILES
  src/internal/inova_registration_views_stepselector_PluginActivator.h
  src/internal/RegistrationStepSelector.h
)

# List of resource files that can be used by the plugin system without loading
# the actual plugin. For example, the icon that is typically displayed in the
# plugin view menu at the top of the application window.
set(CACHED_RESOURCE_FILES
  resources/RegistrationIconI.xpm
  plugin.xml
)

set(QRC_FILES

)
