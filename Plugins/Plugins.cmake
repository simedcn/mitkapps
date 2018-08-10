if("${ProjectToGenerate}" STREQUAL "OrganPrint")
	set(PROJECT_PLUGINS

	  Plugins/my.organprint.app:OFF
	  Plugins/my.organprint.renderwindoweditor:ON
	  Plugins/my.organprint.minimalapplication:ON
	  Plugins/my.organprint.views.stepselector:ON
          Plugins/my.pacs.views.dicomview:ON
          Plugins/my.organprint.views.sidepanel:ON
	)
elseif("${ProjectToGenerate}" STREQUAL "POPE")
	  set(PROJECT_PLUGINS
	    Plugins/my.popeproject.renderwindoweditor:ON
	    Plugins/my.popeproject.toolsplugin:ON
	    Plugins/my.popeproject.segmentation:ON
	    Plugins/my.popeproject.mainapplication:ON
	    Plugins/my.pacs.views.dicomview:ON
	    Plugins/my.popeproject.registration:ON
	  )
endif()
