set(CPP_FILES
  src/internal/my_popeproject_registrationplugin_PluginActivator.cpp
  src/internal/RegistrationPlugin.cpp
  src/internal/RegistrationPluginPreferencePage.cpp
  src/internal/ImageRegistrationCalculationThread.cpp
  src/internal/nrnGlobals.h
  src/internal/nrnGlobals.cpp
)

set(UI_FILES
  src/internal/RegistrationPluginControls.ui
)

set(MOC_H_FILES
  src/internal/my_popeproject_registrationplugin_PluginActivator.h
  src/internal/RegistrationPlugin.h
  src/internal/RegistrationPluginPreferencePage.h
  src/internal/ImageRegistrationCalculationThread.h
)

# List of resource files that can be used by the plugin system without loading
# the actual plugin. For example, the icon that is typically displayed in the
# plugin view menu at the top of the application window.
set(CACHED_RESOURCE_FILES
  resources/RegistrationIcon.xpm
  plugin.xml
)

set(QRC_FILES
  resources/RegistrationPlugin.qrc
)
