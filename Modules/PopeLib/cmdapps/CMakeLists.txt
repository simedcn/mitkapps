option(BUILD_MyPopeCmdApps "Build commandline tools for the pope module" OFF)

if(BUILD_MyPopeCmdApps)

  mitkFunctionCreateCommandLineApp(
    NAME OffsetImageCmdApp
    DEPENDS PopeLib
    PACKAGE_DEPENDS ITK
    )

endif()
