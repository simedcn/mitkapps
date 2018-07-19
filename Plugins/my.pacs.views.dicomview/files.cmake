set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  DicomView.cpp
  DicomViewDialog.cpp
  WindowListener.cpp
  my_pacs_views_dicomview_Activator.cpp
  DicomViewPreferencePage.cpp
  UploadToPACSAction.cpp

)

set(MOC_H_FILES
  src/internal/DicomView.h
  src/internal/WindowListener.h
  src/internal/my_pacs_views_dicomview_Activator.h
  src/internal/DicomViewPreferencePage.h
  src/internal/UploadToPACSAction.h
)

set(UI_FILES
  src/internal/DicomViewControls.ui
)

set(QRC_FILES
  resources/DicomView.qrc
)

set(CACHED_RESOURCE_FILES
  resources/CreateSurface.png
  plugin.xml
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
