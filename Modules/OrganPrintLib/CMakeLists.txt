mitk_create_module(OrganPrintLib
    DEPENDS PUBLIC MitkCore
    PACKAGE_DEPENDS
    PUBLIC Qt5|Core GDCM
)

add_subdirectory(cmdapps)
