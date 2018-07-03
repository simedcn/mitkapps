//#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "ui_MainWindow.h"

#include <QMainWindow>
#include <QApplication>
#include <QHBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <QTextStream>
#include <qfiledialog.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkIOUtil.h>
#include <QmitkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <QmitkRegisterClasses.h>
#include <mitkExtractSliceFilter.h>
#include <mitkPlaneGeometry.h>
#include <mitkLookupTable.h>
#include <vtkMitkLevelWindowFilter.h>
#include <mitkProperties.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkImage.h>
#include <mitkNodePredicateDataType.h>
//#include <QmitkSliceWidget.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkPointSet.h>
#include <mitkPointSetDataInteractor.h>
#include <QmitkStdMultiWidget.h>
#include <QMouseEvent>
#include <mitkSliceNavigationController.h>
#include <mitkInteractionEvent.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkFileReaderSelector.h>
#include <mitkDataStorage.h>
//#include <mitkDataStorageSelection.h>
#include <mitkDataNode.h>
#include <itkImage.h>
#include <mitkPointSet.h>
#include <QSettings>

#include <service/event/ctkEventAdmin.h>

#include <memory>

// Forward declaration of ui widget
namespace Ui
{
	class mainwindow;
}

class MainWindow : public QWidget
{
	Q_OBJECT
public:
	MainWindow(QmitkStdMultiWidget* multiwidget, QWidget* parent = nullptr);
	~MainWindow();

	QmitkStdMultiWidget* getStdMultiWidget();

protected:
	void enable3DRepresentation(bool flag);

signals:
	void ImageSetHasToBeLoaded(const QString& filename);
	void ImageFolderHasToBeLoaded(const QString& filename);
	void EnableAutoRotation(bool);
	void Representation3D_changed(const ctkDictionary&);
	
public slots:
	void on_pushButton_OpenDICOM_clicked();
	void on_pushButton_OpenFolder_clicked();
	void on_pushButton_PACS_clicked();
	void on_pushButton_Settings_clicked();
	void on_pushButton_ViewAll_clicked();

	void On_ToolsPlugin_Representation3DHasToBeInitiated(const ctkEvent& event);
	void On_ToolsPlugin_NodeHasManyImages(const ctkEvent& event);
	void On_ToolsPlugin_SetRange(const ctkEvent& event);

private:
	Ui::mainwindow& ui;
	QmitkStdMultiWidget* multiWidget;
};

#endif // MAINWINDOW_H