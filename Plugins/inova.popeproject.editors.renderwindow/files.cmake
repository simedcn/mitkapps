
SET(INTERNAL_CPP_FILES
  inova_popeproject_editors_renderwindow_Activator.cpp
  PopeRenderWindowEditor.cpp
  MainWindow.cpp
  DataManager.cpp
  PatientSelector.cpp
  PopeSettings.cpp
  PopePreferencePage.cpp
)

SET(SRC_H_FILES
  internal/PopeRenderWindowEditorPrivate.h
  internal/QmitkStdMultiWidgetPartListener.h
)

SET(SRC_CPP_FILES
  internal/PopeRenderWindowEditorPrivate.cpp
  internal/QmitkStdMultiWidgetPartListener.cpp
)

SET(MOC_H_FILES
  src/internal/inova_popeproject_editors_renderwindow_Activator.h
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
  resources/PopeIcon.png
  resources/POPE_icon.ico
  plugin.xml
)

SET(QRC_FILES
  resources/inova_popeproject_editors_renderwindow.qrc
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
