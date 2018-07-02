//#pragma once
#include <QMainWindow>
#include <QApplication>
#include <QHBoxLayout>
#include <QWidget>
#include <QTabWidget>
#include <QTextStream>
#include <qfiledialog.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkIOUtil.h>
#include "QmitkRenderWindow.h"
#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include "QmitkRegisterClasses.h"
#include <mitkExtractSliceFilter.h>
#include "mitkPlaneGeometry.h"
#include <mitkLookupTable.h>
#include <vtkMitkLevelWindowFilter.h>
#include <mitkProperties.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkImage.h>
#include "mitkNodePredicateDataType.h"
//#include "QmitkSliceWidget.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkPointSet.h"
#include "mitkPointSetDataInteractor.h"
#include <QmitkStdMultiWidget.h>
#include <QMouseEvent>
#include <mitkSliceNavigationController.h>
#include "mitkInteractionEvent.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkFileReaderSelector.h"
#include "mitkDataStorage.h"
//#include "mitkDataStorageSelection.h"
#include "mitkDataNode.h"
#include <itkImage.h>
#include <mitkPointSet.h>
#include <QSettings>

#include <memory>

// Forward declaration of ui widget
namespace Ui {
class mainwindow;
}

class MainWindow : public QWidget
{
  Q_OBJECT
public:
  MainWindow(QmitkStdMultiWidget *multiwidget, QWidget *parent = nullptr);
  ~MainWindow();

  QmitkStdMultiWidget* getStdMultiWidget();

signals:
  void loadButton(const QString& filename);

public slots:
  void on_openButton_pressed();

private:

  QmitkStdMultiWidget* m_MultiWidget;

  Ui::mainwindow* ui;
};
