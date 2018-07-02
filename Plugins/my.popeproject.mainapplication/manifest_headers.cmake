set(Plugin-Name "Main POPE Application")
set(Plugin-Version "1.0.0")
set(Plugin-Vendor "Inova DE")
set(Plugin-ContactAddress "http://www.inova-de.eu")
set(Require-Plugin
	org.blueberry.ui.qt
 #	org.mitk.gui.qt.extapplication
	org.mitk.gui.qt.ext
 	#org.mitk.gui.qt.application # Initializes GlobalInteraction and registers MITK Core factories
	#org.mitk.core.ext
#	org.mitk.gui.qt.dicom
#	org.mitk.gui.qt.xnat
)

