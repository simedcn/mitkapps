set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  my_popeproject_mainapplication_Activator.cpp
  MainApplication.cpp
  MainPerspective.cpp
  DicomPerspective.cpp
  MainWorkbenchAdvisor.cpp
  MainApplicationPreferencePage.cpp
  MainWorkbenchWindowAdvisor.cpp
  ShowViewAction.cpp
  PartListenerForPlugins.cpp
)

set(MOC_H_FILES
  src/internal/my_popeproject_mainapplication_Activator.h
  src/internal/MainApplication.h
  src/internal/MainPerspective.h
  src/internal/DicomPerspective.h
  src/internal/MainWorkbenchAdvisor.h
  src/internal/MainApplicationPreferencePage.h
  src/internal/MainWorkbenchWindowAdvisor.h
  src/internal/ShowViewAction.h
  src/internal/PartListenerForPlugins.h
)

set(CACHED_RESOURCE_FILES
  resources/POPE_icon.ico
  resources/logo48.png
  plugin.xml
)

set(QRC_FILES
  resources/my_popeproject_mainapplication.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
