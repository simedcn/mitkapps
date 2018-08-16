set(CPP_FILES
  src/internal/inova_popeproject_views_segmentation_PluginActivator.cpp
  src/internal/SegmentationView.cpp
)

set(UI_FILES
  src/internal/SegmentationView.ui
)

set(MOC_H_FILES
  src/internal/inova_popeproject_views_segmentation_PluginActivator.h
  src/internal/SegmentationView.h
)

# List of resource files that can be used by the plugin system without loading
# the actual plugin. For example, the icon that is typically displayed in the
# plugin view menu at the top of the application window.
set(CACHED_RESOURCE_FILES
  resources/SegmentationIcon.png
  plugin.xml
)

set(QRC_FILES
  resources/SegmentationView.qrc
)
