SET(SRC_CPP_FILES
)

SET(INTERNAL_CPP_FILES
  my_popeproject_renderwindoweditor_Activator.cpp
  PopeRenderWindowEditor.cpp
  MainWindow.cpp
  DataManager.cpp
  PatientSelector.cpp
  PopeSettings.cpp
  PopePreferencePage.cpp
)

SET(SRC_CPP_FILES
  internal/PopeRenderWindowEditorPrivate.h
  internal/PopeRenderWindowEditorPrivate.cpp
  internal/QmitkStdMultiWidgetPartListener.h
  internal/QmitkStdMultiWidgetPartListener.cpp
)

SET(MOC_H_FILES
  src/internal/my_popeproject_renderwindoweditor_Activator.h
  src/internal/PopeRenderWindowEditor.h
  src/internal/MainWindow.h
  src/internal/DataManager.h
  src/internal/PatientSelector.h
  src/internal/PopeSettings.h
  src/internal/PopePreferencePage.h
)

SET(UI_FILES
  src/internal/MainWindow.ui
  src/internal/PatientSelector.ui
  src/internal/PopeSettings.ui
)

SET(CACHED_RESOURCE_FILES
  plugin.xml
)

SET(QRC_FILES
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
