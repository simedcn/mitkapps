set(SRC_CPP_FILES
)

set(INTERNAL_CPP_FILES
  my_organprint_minimalapplication_Activator.cpp
  MinimalApplication.cpp
  MinimalPerspective.cpp
)

set(MOC_H_FILES
  src/internal/my_organprint_minimalapplication_Activator.h

  src/internal/MinimalApplication.h
  src/internal/MinimalPerspective.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(QRC_FILES
  resources/my_awesomeproject_minimalapplication.qrc
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
