project(${MY_APP_NAME})

set(_app_options)
if(${MY_PROJECT_NAME}_SHOW_CONSOLE_WINDOW)
  list(APPEND _app_options SHOW_CONSOLE)
endif()

# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins
)

# set(SOURCE_FILES
#   POPEApp.cpp
#   ../../Plugins/inova.popeproject.apps.mainapplication/resources/app.rc 
# )

mitkFunctionCreateBlueBerryApplication(
  NAME ${MY_APP_NAME}
  DESCRIPTION "MITK - POPEApp Application"
#   SOURCES  ${SOURCE_FILES}
  EXCLUDE_PLUGINS ${_exclude_plugins}
  ${_app_options}
)
mitk_use_modules(TARGET ${MY_APP_NAME}
  MODULES MitkAppUtil MitkSegmentation MitkQtWidgetsExt MitkSegmentationUI
  PACKAGES Qt5|Widgets
)
