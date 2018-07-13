#include "ui_MainWindow.h"

#include "MainWindow.h"
#include "mitkImageAccessByItk.h"
#include "QmitkRenderWindow.h"
//#include "QmitkSliceWidget.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkPointSet.h"
#include "mitkPointSetDataInteractor.h"
#include "mitkImageAccessByItk.h"
#include "mitkRenderingManager.h"
#include <mitkIOUtil.h>
#include <mitkDataStorage.h>

#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMFileReader.h>
#include <mitkDICOMTagsOfInterestHelper.h>
#include <mitkDICOMProperty.h>
#include <mitkDICOMFilesHelper.h>
#include <mitkDICOMITKSeriesGDCMReader.h>
#include <itkGDCMImageIO.h>
#include <itksys/SystemTools.hxx>
#include <gdcmDirectory.h>
#include <QString>
#include <QMessageBox>
#include <ctkDictionary.h>



MainWindow::MainWindow(QmitkStdMultiWidget* multiWidget, QWidget *parent) :
    QWidget(parent),
    m_MultiWidget(multiWidget),
    ui(new Ui::mainwindow)
{
    ui->setupUi(this);
    //Register Qmitk-dependent global instances
    QmitkRegisterClasses();

    //m_MultiWidget->setStyleSheet("background-color:black; color: #FFF; border:0px");
    // Tell the multiWidget which DataStorage to render

    ui->Patient_display_layout->addWidget(m_MultiWidget);

    //  bool flag = true;




}

MainWindow::~MainWindow()
{
    delete this->m_MultiWidget;
    delete ui;
}

QmitkStdMultiWidget *MainWindow::getStdMultiWidget()
{
    return this->m_MultiWidget;
}

//LOAD DATA FROM EXPLORER
void MainWindow::on_openButton_pressed()
{
    QString imagePath = QFileDialog::getOpenFileName(
                            this, tr("Open File")
                        );
    emit this->loadButton(imagePath);
}
