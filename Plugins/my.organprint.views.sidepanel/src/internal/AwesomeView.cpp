/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "AwesomeView.h"

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <usModuleRegistry.h>
#include <QmitkDataStorageComboBox.h>

#include <QMessageBox>
#include <QPainter>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>

#include <AwesomeImageFilter.h>
#include <AwesomeImageInteractor.h>

#include <vtkSTLWriter.h>

#include "mitkLabel.h"
#include "mitkToolManagerProvider.h"
#include "mitkOtsuSegmentationFilter.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkIOUtil.h"

// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModuleResource.h"

#include "itkImageRegionIterator.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>
#include <mitkPaintbrushTool.h>

#include <itkBinaryThresholdImageFilter.h>

// Don't forget to initialize the VIEW_ID.
const std::string AwesomeView::VIEW_ID = "my.awesomeproject.views.awesomeview";

void AwesomeView::CreateQtPartControl(QWidget* parent)
{
    // Setting up the UI is a true pleasure when using .ui files, isn't it?
    m_Controls.setupUi(parent);
    this->GetRenderWindowPart(OPEN);
    this->RequestRenderWindowUpdate();
    // Wire up the UI widgets with our functionality.

    m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    assert(m_ToolManager);

    SetLabelWidget();

    // Connect Signals and Slots of the Plugin UI
    connect(this->m_Controls.STLsExport_button, SIGNAL(clicked()), this, SLOT(OnSTLsExportButtonClicked()));
    connect(this->m_Controls.pushButtonOtsuSegmentation, SIGNAL(clicked()), this, SLOT(OnPushButtonOtsuSegmentationClicked()));
    connect(this->m_Controls.pbExportCSV, SIGNAL(clicked()), this, SLOT(OnExportButtonClicked()));
    connect(this->m_Controls.m_VolRenderingButton, SIGNAL(toggled(bool)), this, SLOT(enableVolRenderingForCurrentNode(bool)));
    connect(this->m_Controls.paintButton, SIGNAL(clicked()), this, SLOT(enablePaint()));


    // Create the DisplayCoordinate Supplier and provide it with the functioname it should call
    m_DisplayCoordinateSupplier = std::make_unique<DisplayCoordinateSupplier>();
    QString functionName = "NotifyCoordinates";
    m_DisplayCoordinateSupplier->RegisterForUpdates(this, functionName);
    // register the coordinate supplier as MicroService
    this->m_ServiceRegistration = us::GetModuleContext()->RegisterService<mitk::InteractionEventObserver>(this->m_DisplayCoordinateSupplier.get());
}

AwesomeView::AwesomeView() : m_ActiveWorkingNode(nullptr)
{
}

AwesomeView::~AwesomeView()
{
    this->m_ServiceRegistration.Unregister();
}

void AwesomeView::SetFocus()
{

}

void AwesomeView::SetLabelWidget()
{
    QTableWidget *tableWidged = m_Controls.m_LabelSetTableWidget;

    tableWidged->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    tableWidged->setTabKeyNavigation(false);
    tableWidged->setAlternatingRowColors(false);
    tableWidged->setFocusPolicy(Qt::NoFocus);
    tableWidged->setColumnCount(3);
    tableWidged->resizeColumnToContents(NAME_COL);
    tableWidged->setColumnWidth(COLOR_COL, 50);
    tableWidged->setColumnWidth(VISIBLE_COL, 50);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    tableWidged->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
    tableWidged->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
#endif
    tableWidged->setContextMenuPolicy(Qt::CustomContextMenu);
    tableWidged->horizontalHeader()->hide();
    tableWidged->setSortingEnabled(false);
    tableWidged->verticalHeader()->hide();
    tableWidged->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidged->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tableWidged->setSelectionBehavior(QAbstractItemView::SelectRows);

}

void AwesomeView::OnSTLsExportButtonClicked() {
    MITK_INFO << "Export STLs";
    itk::SmartPointer<mitk::ImageToSurfaceFilter> i2sf = mitk::ImageToSurfaceFilter::New();
    itk::SmartPointer<mitk::Surface> surface = mitk::Surface::New();
    //vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();

    if (m_Controls.m_LabelSetTableWidget->rowCount() <= 0)
        return;

    QString dir = QFileDialog::getExistingDirectory(nullptr, tr("Open Directory"),
                  QString(), // standard path
                  QFileDialog::ShowDirsOnly
                  | QFileDialog::DontResolveSymlinks);

    QString csvPath = dir + "/segmentation.csv";
    QFile data(csvPath);
    if (!data.open(QFile::WriteOnly | QFile::Truncate))
        return;

    for (int row = 0; row < m_Controls.m_LabelSetTableWidget->rowCount(); ++row)
    {
        std::string name_selected = m_Controls.m_LabelSetTableWidget->item(row, 0)->data(Qt::UserRole).toString().toStdString();
        mitk::DataNode* node_selected = GetDataStorage()->GetNamedNode(name_selected);
        std::string name;
        node_selected->GetStringProperty("name", name);
        MITK_INFO << name;

        i2sf->SetInput(dynamic_cast<mitk::Image*> (node_selected->GetData()));
        i2sf->Update();
        surface = i2sf->GetOutput();
        MITK_INFO << surface->GetVtkPolyData()->GetNumberOfPoints();

        std::stringstream sstr;
        sstr << dir.toStdString() << "/" << name << ".stl";
        MITK_INFO << sstr.str();

        float rgb[3];
        node_selected->GetColor(rgb);
        QTextStream output(&data);
        output << dir << "/" << QString::fromStdString(name) << ".stl; #" << QString("%1%2%3").arg(int(rgb[0] * 255), 2, 16, QChar('0')).arg(int(rgb[1] * 255), 2, 16, QChar('0')).arg(int(rgb[2] * 255), 2, 16, QChar('0')).toUpper();

        mitk::IOUtil::Save(surface, sstr.str());
        /*writer->SetFileName(sstr.str().c_str());
        writer->SetInputData(surface->GetVtkPolyData());
        writer->Update();*/
    }
}


void AwesomeView::OnPushButtonOtsuSegmentationClicked()
{
    if (!m_ActiveWorkingNode)
        return;
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_ActiveWorkingNode->GetData());
    if (!image)
        return;
    // now that we have the image, there are different ways to perform image filters on it
    // Here using an MITK image filter

    //binärbild (segmentierung) auswählen, zb im datamanager oder per combobox
    // image mit segmentierung maskieren und ergebnis -> otsufilter->SetInput(maskedImage);
    mitk::OtsuSegmentationFilter::Pointer otsuFilter = mitk::OtsuSegmentationFilter::New();
    otsuFilter->SetNumberOfThresholds(3);
    otsuFilter->SetValleyEmphasis(false);
    otsuFilter->SetNumberOfBins(255);
    otsuFilter->SetInput(image);

    try
    {
        otsuFilter->Update();
    }
    catch (...)
    {
        mitkThrow() << "itkOtsuFilter error (image dimension must be in {2, 3} and image must not be RGB)";
    }


    typedef itk::Image<unsigned char, 3> TImage;

    mitk::Image* img = otsuFilter->GetOutput();
    // Create a new ITK image and assign the content of the MITK image to it
    TImage::Pointer itkImg = TImage::New();
    mitk::CastToItkImage(img,itkImg);
    // Not we can put the image into an ITK Filter

    typedef itk::BinaryThresholdImageFilter<TImage, TImage> FilterType;
    typename FilterType::Pointer filter = FilterType::New();
    // set properties for filter
    filter->SetInput(itkImg);

    // accept values 1 to 2
    filter->SetLowerThreshold(1);
    filter->SetUpperThreshold(2);
    filter->SetInsideValue(1);
    filter->SetOutsideValue(0);
    filter->Update();

    typename TImage::Pointer itkBinaryResultImage;
    mitk::Image::Pointer binarySegmentation;

    itkBinaryResultImage = filter->GetOutput();
    mitk::CastToMitkImage(itkBinaryResultImage, binarySegmentation);

    // Create a new DataNode to put the output image in;
    mitk::DataNode::Pointer resultImage = mitk::DataNode::New();
    resultImage->SetData(binarySegmentation);
    resultImage->SetName("Binary Otsu Result");
    this->GetDataStorage()->Add(resultImage);
}


void AwesomeView::OnColorButtonClicked()
{
    int row = -1;
    for (int i = 0; i < m_Controls.m_LabelSetTableWidget->rowCount(); ++i)
    {
        if (sender() == m_Controls.m_LabelSetTableWidget->cellWidget(i, COLOR_COL))
        {
            row = i;
        }
    }

    if (row >= 0 && row < m_Controls.m_LabelSetTableWidget->rowCount())
    {
        std::string name_selected = m_Controls.m_LabelSetTableWidget->item(row, 0)->data(Qt::UserRole).toString().toStdString();
        //std::string name_selected = m_Controls.m_LabelSetTableWidget->item(row, 0)->text().toStdString();
        float rgb[3];
        mitk::DataNode* node_selected = GetDataStorage()->GetNamedNode(name_selected);
        node_selected->GetColor(rgb);
        mitk::Color color;
        color.SetRed(rgb[0]);
        color.SetGreen(rgb[1]);
        color.SetBlue(rgb[2]);
        QColor initial(color.GetRed() * 255, color.GetGreen() * 255, color.GetBlue() * 255);
        QColor qcolor = QColorDialog::getColor(initial, 0, QString("Change color"));
        if (!qcolor.isValid())
        {
            return;
        }

        QPushButton *button = static_cast<QPushButton*>(m_Controls.m_LabelSetTableWidget->cellWidget(row, COLOR_COL));
        if (!button)
        {
            return;
        }

        button->setAutoFillBackground(true);

        QString styleSheet = "background-color:rgb(";
        styleSheet.append(QString::number(qcolor.red()));
        styleSheet.append(",");
        styleSheet.append(QString::number(qcolor.green()));
        styleSheet.append(",");
        styleSheet.append(QString::number(qcolor.blue()));
        styleSheet.append(")");
        button->setStyleSheet(styleSheet);

        mitk::Color newColor;
        newColor.SetRed(qcolor.red() / 255.0);
        newColor.SetGreen(qcolor.green() / 255.0);
        newColor.SetBlue(qcolor.blue() / 255.0);

        node_selected->SetColor(newColor);
    }
}

void AwesomeView::OnVisibleButtonClicked()
{
    int row = -1;
    for (int i = 0; i < m_Controls.m_LabelSetTableWidget->rowCount(); ++i)
    {
        if (sender() == m_Controls.m_LabelSetTableWidget->cellWidget(i, VISIBLE_COL))
        {
            row = i;
            break;
        }
    }

    if (row >= 0 && row < m_Controls.m_LabelSetTableWidget->rowCount())
    {
        //QTableWidgetItem *item = m_Controls.m_LabelSetTableWidget->item(row, 0);
        std::string name_selected = m_Controls.m_LabelSetTableWidget->item(row, 0)->data(Qt::UserRole).toString().toStdString();
        mitk::DataNode* node_selected = GetDataStorage()->GetNamedNode(name_selected);
        bool visible = false;
        node_selected->GetVisibility(visible,nullptr);
        node_selected->SetVisibility(!visible);
    }
}

void AwesomeView::OnExportButtonClicked()
{
    QString imagePath = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save to folder ..."),"",tr("CSV Export (*.csv);"));
    // Exports image values into CSV
    // x,y,z, Segmentation_value, Modality1_Value, Modality2_Value,Modality3_Value,...

    QFile data(imagePath);
    if (!data.open(QFile::WriteOnly | QFile::Truncate))
        return;

    QTextStream output(&data);

    QModelIndex segmentationSelection = m_Controls.m_LabelSetTableWidget->currentIndex();
    QString segmentationName = segmentationSelection.data(Qt::DisplayRole).toString();

    QModelIndex index = m_Controls.listWidget->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    MITK_INFO << itemText;

    QString header = "x,y,z";
    header = header + "," + segmentationName;
    for(int row = 0; row < m_Controls.listWidget->count(); row++)
    {
        QListWidgetItem *item = m_Controls.listWidget->item(row);
        header = header + "," + item->text();
    }
    output << header << "\n";

    // Now iterate through image and write down all voxel positions and image values
    // Here we use ITK to do this
    typedef itk::Image<unsigned char, 3> TSegmentationImage;
    typedef itk::Image<double, 3> TFeatureImage;
    typedef TFeatureImage::Pointer PTFeatureImage;


    mitk::Image* img = dynamic_cast<mitk::Image*> (GetDataStorage()->GetNamedNode(segmentationName.toStdString())->GetData());
    // Create a new ITK image and assign the content of the MITK image to it
    TSegmentationImage::Pointer itkImg = TSegmentationImage::New();
    mitk::CastToItkImage(img,itkImg);

    std::vector<PTFeatureImage> featImgs;
    for(int row = 0; row < m_Controls.listWidget->count(); row++)
    {
        QListWidgetItem *item = m_Controls.listWidget->item(row);

        mitk::Image* img = dynamic_cast<mitk::Image*> (GetDataStorage()->GetNamedNode(item->text().toStdString())->GetData());
        PTFeatureImage itkImg = TFeatureImage::New();
        mitk::CastToItkImage(img,itkImg);
        featImgs.push_back(itkImg);
    }

    // Use an itk image iterator to walk through all voxel of the segmentation
    itk::ImageRegionIterator<TSegmentationImage> it(itkImg, itkImg->GetLargestPossibleRegion());
    while (!it.IsAtEnd())
    {
        QString voxelValues;
        if (it.Get() != 0)
        {
            // the position of the iterator in index position
            TSegmentationImage::IndexType index =  it.GetIndex();
            // using the segmentation images geometry the index is mapped to a point in world coordinates
            itk::Point<double, 3> worldPos;
            itkImg->TransformIndexToPhysicalPoint(index, worldPos);
            voxelValues = voxelValues + QString::number(worldPos[0]) + "," + QString::number(worldPos[1]) + "," + QString::number(worldPos[2]);
            voxelValues = voxelValues + "," + QString::number(it.Get());
            // mapping the world coordinates to index coordinates for each image separately
            // NOTE: this is only necesary when we cannot be sure that all images match perfectly
            // Index Coordinates can be used directly if the following is given:
            // - Images all have the same dimensions, i.e. same resolution in each direction AND
            // - origin, spacing and orientation are exactly the same.
            for (auto featureImg : featImgs )
            {
                TFeatureImage::IndexType fIndex;
                featureImg->TransformPhysicalPointToIndex(worldPos, fIndex);
                // check if this position exists in the current image
                if (featureImg->GetLargestPossibleRegion().IsInside(fIndex))
                    voxelValues = voxelValues + "," + QString::number(featureImg->GetPixel(fIndex));
                else
                    voxelValues = voxelValues + ",0";
            }
            output << voxelValues << "\n";
        }
        ++it;
    }
}


void AwesomeView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& dataNodes)
{
    m_Controls.m_VolRenderingButton->setEnabled(false);
    m_VRNode = nullptr;
    if(dataNodes.size()==1)
    {
        mitk::Image* img = dynamic_cast<mitk::Image*>(dataNodes.first()->GetData());
        if(img)
        {
            m_VRNode = dataNodes.first();
            m_Controls.m_VolRenderingButton->setEnabled(true);
            bool vrEnabled = false;
            m_VRNode->GetBoolProperty("volumerendering",vrEnabled);
            m_Controls.m_VolRenderingButton->setChecked(vrEnabled);
        }
    }
    for (const auto& dataNode : dataNodes)
    {
        // Write robust code. Always check pointers before using them. If the
        // data node pointer is null, the second half of our condition isn't
        // even evaluated and we're safe (C++ short-circuit evaluation).
        if (dataNode.IsNotNull() && dynamic_cast<mitk::Image*>(dataNode->GetData()) != nullptr)
        {
            m_Controls.selectImageLabel->setVisible(false);

            // Remeber activated datanode, selected in DataStorage
            m_ActiveWorkingNode = dataNode;

            std::string imgName;
            dataNode->GetStringProperty("name", imgName);

            SetLabelWidget();

            if (dataNode->GetData()->GetProperty("DICOM.0010.0010") &&
                    dataNode->GetData()->GetProperty("DICOM.0010.0030") &&
                    dataNode->GetData()->GetProperty("DICOM.0010.0040") &&
                    dataNode->GetData()->GetProperty("DICOM.0010.0020") )
            {
                std::string patientName = dataNode->GetData()->GetProperty("DICOM.0010.0010")->GetValueAsString();
                std::string patientBirthdate = dataNode->GetData()->GetProperty("DICOM.0010.0030")->GetValueAsString();
                std::string patientGender = dataNode->GetData()->GetProperty("DICOM.0010.0040")->GetValueAsString();
                std::string patientID = dataNode->GetData()->GetProperty("DICOM.0010.0020")->GetValueAsString();

                m_Controls.PatientName_label->setText(QString::fromStdString(patientName));
                m_Controls.PatientDOB_label->setText(QString::fromStdString(patientBirthdate));
                m_Controls.PatientGender_label->setText(QString::fromStdString(patientGender));
                m_Controls.PatientID_label->setText(QString::fromStdString(patientID));
            }
            else if (dataNode->GetData()->GetProperty("dicom.patient.PatientsName") &&
                     dataNode->GetData()->GetProperty("dicom.patient.PatientsBirthDate") &&
                     dataNode->GetData()->GetProperty("dicom.patient.PatientsSex") &&
                     dataNode->GetData()->GetProperty("dicom.patient.PatientID") )
            {
                std::string patientName = dataNode->GetData()->GetProperty("dicom.patient.PatientsName")->GetValueAsString();
                std::string patientBirthdate = dataNode->GetData()->GetProperty("dicom.patient.PatientsBirthDate")->GetValueAsString();
                std::string patientGender = dataNode->GetData()->GetProperty("dicom.patient.PatientsSex")->GetValueAsString();
                std::string patientID = dataNode->GetData()->GetProperty("dicom.patient.PatientID")->GetValueAsString();

                m_Controls.PatientName_label->setText(QString::fromStdString(patientName));
                m_Controls.PatientDOB_label->setText(QString::fromStdString(patientBirthdate));
                m_Controls.PatientGender_label->setText(QString::fromStdString(patientGender));
                m_Controls.PatientID_label->setText(QString::fromStdString(patientID));
            }
            else
            {
                MITK_INFO << "Reading DICOM Tags failed.";
                m_Controls.PatientName_label->setText("-");
                m_Controls.PatientDOB_label->setText("-");
                m_Controls.PatientGender_label->setText("-");
                m_Controls.PatientID_label->setText("-");
            }
            return;
        }
    }


    // Nothing is selected or the selection doesn't contain an image.
    m_Controls.selectImageLabel->setVisible(true);
}


void AwesomeView::NotifyCoordinates(QVector<double> point)
{


    // TODO: exlude binary images !!!!!
    // !!!!!!!!
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateNot::Pointer isNotVisible = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("visible", mitk::BoolProperty::New(false)));

    mitk::NodePredicateAnd::Pointer validVisibleImage = mitk::NodePredicateAnd::New(isImage, isNotVisible);

    // Parse all visible images and list the values of the clicked position
    mitk::DataStorage::SetOfObjects::ConstPointer patientImages = this->GetDataStorage()->GetSubset(validVisibleImage);


    mitk::Point3D p3d;
    p3d[0] = point[0];
    p3d[1] = point[1];
    p3d[2] = point[2];

    QString valueText = "Pixel values(s): ";
    for (mitk::DataStorage::SetOfObjects::ConstIterator iter = patientImages->Begin(); iter != patientImages->End(); ++iter)
    {

        if (iter->Value().IsNotNull() && dynamic_cast<mitk::Image*>(iter->Value()->GetData()) != nullptr)
        {
            std::string imgName;
            iter->Value()->GetStringProperty("name", imgName);

            mitk::Image* img = dynamic_cast<mitk::Image*>(iter->Value()->GetData());
            double value = img->GetPixelValueByWorldCoordinate(p3d);

            valueText = valueText + QString::fromStdString(imgName) + ": " + QString::number(value) + "\n()";

            m_Controls.pixelValue->setText(valueText);
        }
    }
}

void AwesomeView::enableVolRenderingForCurrentNode(bool volRen)
{
    m_VRNode->SetBoolProperty("volumerendering",volRen);
}

void AwesomeView::enablePaint()
{
    auto tm = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
//  tm->SetReferenceData(const_cast<mitk::DataNode*>(referenceData));
//  tm->SetWorkingData(const_cast<mitk::DataNode*>(workingData));
    tm->ActivateTool(tm->GetToolIdByToolType<mitk::PaintbrushTool>());
}

void AwesomeView::NodeRemoved(const mitk::DataNode *node)
{
    QTableWidget* tableWidget = m_Controls.m_LabelSetTableWidget;
    for(int i = 0 ; i<tableWidget->rowCount(); ++i)
    {
        if (tableWidget->item(i,NAME_COL)->text().toStdString().compare(node->GetName()) == 0)
        {
            tableWidget->removeRow(i);
            return;
        }
    }

}

void AwesomeView::NodeAdded(const mitk::DataNode *node)
{
    bool binary = false;
    node->GetBoolProperty("binary", binary);
    bool segmentation = false;
    node->GetBoolProperty("binary", segmentation);
    if (!binary || !segmentation)
    {
        mitk::Image* img = dynamic_cast<mitk::Image*> (node->GetData());
        if (img)
        {
            m_Controls.listWidget->addItem(new QListWidgetItem(QString::fromStdString(node->GetName())));
        }
        return;
    }


    itk::SmartPointer<mitk::Label> label = mitk::Label::New();
    std::string name;
    node->GetStringProperty("name", name);
    label->SetName(name);

    float rgb[3];
    node->GetColor(rgb);
    mitk::Color color;
    color.SetRed(rgb[0]);
    color.SetGreen(rgb[1]);
    color.SetBlue(rgb[2]);

    QString styleSheet = "background-color:rgb(";
    styleSheet.append(QString::number(color[0] * 255));
    styleSheet.append(",");
    styleSheet.append(QString::number(color[1] * 255));
    styleSheet.append(",");
    styleSheet.append(QString::number(color[2] * 255));
    styleSheet.append(")");

    QTableWidget* tableWidget = m_Controls.m_LabelSetTableWidget;
    int colWidth = (tableWidget->columnWidth(NAME_COL) < 180) ? 180 : tableWidget->columnWidth(NAME_COL) - 2;
    QString text = tableWidget->fontMetrics().elidedText(label->GetName().c_str(), Qt::ElideMiddle, colWidth);
    QTableWidgetItem *nameItem = new QTableWidgetItem(text);
    nameItem->setTextAlignment(Qt::AlignCenter | Qt::AlignLeft);
    // ---!---
    // IMPORTANT: ADD PIXELVALUE TO TABLEWIDGETITEM.DATA

    nameItem->setData(Qt::UserRole, QVariant(QString::fromStdString(name)));
    // ---!---

    QPushButton *pbColor = new QPushButton(tableWidget);
    pbColor->setFixedSize(24, 24);
    pbColor->setCheckable(false);
    pbColor->setAutoFillBackground(true);
    pbColor->setToolTip("Change label color");
    pbColor->setStyleSheet(styleSheet);

    connect(pbColor, SIGNAL(clicked()), this, SLOT(OnColorButtonClicked()));

    QPushButton *pbVisible = new QPushButton(tableWidget);
    pbVisible->setAutoRepeat(false);
    QIcon *iconVisible = new QIcon();
    iconVisible->addFile(QString::fromUtf8(":/Awesome/visible.png"), QSize(), QIcon::Normal, QIcon::Off);
    iconVisible->addFile(QString::fromUtf8(":/Awesome/invisible.png"), QSize(), QIcon::Normal, QIcon::On);
    pbVisible->setIcon(*iconVisible);
    pbVisible->setIconSize(QSize(24, 24));
    pbVisible->setCheckable(true);
    pbVisible->setToolTip("Show/hide segmentation");
    pbVisible->setChecked(!label->GetVisible());

    connect(pbVisible, SIGNAL(clicked()), this, SLOT(OnVisibleButtonClicked()));

    int row = tableWidget->rowCount();
    tableWidget->insertRow(row);
    tableWidget->setRowHeight(row, 24);
    tableWidget->setItem(row, NAME_COL, nameItem);
    tableWidget->setCellWidget(row, COLOR_COL, pbColor);
    tableWidget->setCellWidget(row, VISIBLE_COL, pbVisible);
    tableWidget->selectRow(row);

    // m_LabelSetImage->SetActiveLabel(label->GetPixelValue());
    // m_ToolManager->WorkingDataModified.Send();
    // emit activeLabelChanged(label->GetPixelValue());

    if (tableWidget->rowCount() == 0)
    {
        tableWidget->hideRow(row); // hide exterior label
    }
}

mitk::DataNode *AwesomeView::GetWorkingNode()
{
    mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
    assert(workingNode);
    return workingNode;
}
