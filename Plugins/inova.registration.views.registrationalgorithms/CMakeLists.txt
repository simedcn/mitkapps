project(inova_registration_views_registrationalgorithms)

mitk_create_plugin(
  EXPORT_DIRECTIVE inova_registration_views_registrationalgorithms_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkMatchPointRegistration MitkMatchPointRegistrationUI PopeLib
  PACKAGE_DEPENDS CTK|CTKCore
)

# MatchPoint libraries.
install(DIRECTORY "${MITK_DIR}/../ep/src/MatchPoint-build/bin/Release/" DESTINATION "bin/MatchPoint" FILES_MATCHING PATTERN "*.dll")