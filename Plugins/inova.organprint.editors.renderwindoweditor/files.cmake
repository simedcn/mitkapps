SET(SRC_CPP_FILES
)

SET(INTERNAL_CPP_FILES
  inova_organprint_editors_renderwindoweditor_Activator.cpp
  OrganPrintRenderWindowEditor.cpp
  MainWindow.cpp
  DataManager.cpp
  PatientSelector.cpp
)

SET(MOC_H_FILES
  src/internal/inova_organprint_editors_renderwindoweditor_Activator.h
  src/internal/OrganPrintRenderWindowEditor.h
  src/internal/MainWindow.h
  src/internal/DataManager.h
  src/internal/PatientSelector.h
)

SET(UI_FILES
  src/internal/MainWindow.ui
  src/internal/PatientSelector.ui
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
