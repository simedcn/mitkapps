set(module_dirs
	
  OrganPrintLib
  PopeLib
  Interactions
)

foreach(module_dir ${module_dirs})
  add_subdirectory(${module_dir})
endforeach()
