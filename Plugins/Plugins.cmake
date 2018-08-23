if("${ProjectToGenerate}" STREQUAL "OrganPrint")
	set(PROJECT_PLUGINS

	  Plugins/my.organprint.app:OFF
	  Plugins/my.organprint.renderwindoweditor:ON
	  Plugins/my.organprint.minimalapplication:ON
	  Plugins/my.organprint.views.stepselector:ON
          Plugins/inova.pacs.views.dicomview:ON
          Plugins/my.organprint.views.sidepanel:ON
	)
elseif("${ProjectToGenerate}" STREQUAL "POPE")
	  set(PROJECT_PLUGINS
		Plugins/inova.popeproject.editors.renderwindow:ON
		Plugins/inova.popeproject.views.tools:ON
		Plugins/inova.popeproject.views.segmentation:ON
		Plugins/inova.popeproject.apps.mainapplication:ON
		Plugins/inova.pacs.views.dicomview:ON
		Plugins/inova.registration.views.rigidregistration:ON
		Plugins/inova.registration.views.manualregistration:ON
		Plugins/inova.registration.views.registrationalgorithms:ON
		Plugins/inova.registration.views.frameregistration:ON
		Plugins/inova.registration.views.mapper:ON
		Plugins/inova.registration.views.comparison:ON
		Plugins/inova.registration.views.visualizer:ON
		Plugins/inova.registration.views.stepselector:ON
	  )
endif()
