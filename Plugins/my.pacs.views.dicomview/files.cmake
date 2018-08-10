set(SRC_CPP_FILES
    DicomViewDialog.cpp
)

set(INTERNAL_CPP_FILES
  WindowListener.cpp
  my_pacs_views_dicomview_Activator.cpp
  DicomViewPreferencePage.cpp
  UploadToPACSAction.cpp
  DicomView.cpp
  DicomViewConstants.h
)

set(MOC_H_FILES
  src/internal/DicomView.h
  src/DicomViewDialog.h
  src/internal/WindowListener.h
  src/internal/my_pacs_views_dicomview_Activator.h
  src/internal/DicomViewPreferencePage.h
  src/internal/UploadToPACSAction.h
)

set(UI_FILES
  src/internal/DicomViewControls.ui
  src/internal/DicomViewPreferences.ui
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
