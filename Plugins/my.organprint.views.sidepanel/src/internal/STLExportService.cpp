#include "STLExportService.h"
#include <mitkSurface.h>
#include <mitkImageToSurfaceFilter.h>
#include <QTextStream>
#include <mitkIOUtil.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateFirstLevel.h>
#include <QFile>
#include "itkImageRegionIterator.h"
#include "mitkImageCast.h"
#include "ExternalShapeSmoother.h"

STLExportService::STLExportService()
{

}

bool STLExportService::exportTo(QString & dir, const DataNode * dataNode) {

    QString csvPath = dir + "/segmentation.csv";
    QFile data(csvPath);
    if (!data.open(QFile::WriteOnly | QFile::Truncate))
        return false;

    MITK_INFO << "Export STLs";
    itk::SmartPointer<mitk::ImageToSurfaceFilter> i2sf = mitk::ImageToSurfaceFilter::New();
    itk::SmartPointer<mitk::Surface> surface = mitk::Surface::New();

    ExternalShapeSmoother smoother;



    smoother.smooth(dataNode);
    DataNode * smoothed = smoother.GetResult();

    const DataNode * node_selected = dataNode;
    std::string name = dataNode->GetName();
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

    //surface = (mitk::Surface*) smoothed->GetData();
    if(smoother.Success())
        mitk::IOUtil::Save(smoothed->GetData(), sstr.str());



    //writer->SetFileName(sstr.str().c_str());
    //writer->SetInputData(surface->GetVtkPolyData());
    //writer->Update();
    return true;

}
void STLExportService::SetDataStorage(DataStorage *storage) {
    m_DataStorage = storage;
}
int STLExportService::exportTo(QString & path, DataStorage * storage) {

    cout << "Exporting to " << path.toStdString() << endl;

    const DataNode * selection = GetSelectedNode(storage);

    // gets all the segmented area of the data storage depending on the selection
    SetOfObjects::ConstPointer exportList = GetNodesToExport(storage,selection);

    if(!exportList) {
        return false;
    }

    int count = 0;
    const DataNode * dataNode;
    SetOfObjects::ConstIterator it = exportList->Begin();
    while(it != exportList->End()) {
        dataNode = it->Value();
        exportTo(path,dataNode);
        std::stringstream outputFile;
        outputFile << path.toStdString() << "/" << dataNode->GetName() << ".csv";
        // export the value of all the visible pixels inside the selection
        QString finalPath = QString::fromStdString(outputFile.str());
        exportToCsv(finalPath,storage,dataNode);
        it++;
        count++;
    }
    return (int) exportList->Size();

}



const mitk::DataNode * STLExportService::GetSelectedNode(DataStorage * storage) {
    //const IsSelectedPredicate * isSelected = new IsSelectedPredicate();

    auto isSelected = mitk::NodePredicateProperty::New("selected", mitk::BoolProperty::New(true));

    return storage->GetNode(isSelected);
}

const mitk::DataNode * STLExportService::GetParentNode(DataStorage *storage, const DataNode *node) {

    SetOfObjects::ConstPointer sources = storage->GetSources(node);

    if(sources->Size() ==0) {
        return nullptr;

    }
    else {
        return sources->Begin()->Value();
    }
}
STLExportService::SetOfObjects::ConstPointer STLExportService::GetNodesToExport(DataStorage *storage, const DataNode * selection) {



    if(!storage || !selection) {
        return nullptr;
    }

    if(isParent(storage,selection)) {
        return storage->GetDerivations(selection);
    }

    else {
        return GetNodesToExport(storage,GetParentNode(storage,selection));
    }

}


bool STLExportService::isParent(DataStorage * storage, const DataNode*node) {
    return GetParentNode(storage,node) == nullptr;
}

void STLExportService::printName(const DataNode * node) {
    MITK_DEBUG << "Node [" << &node << "] " << node->GetName() << endl;
}
void STLExportService::Log(const char msg[]) {
    MITK_DEBUG << "[STLExportService] " << msg << endl;
}

void STLExportService::exportToCsv(QString & imagePath, DataStorage * storage, const DataNode * node) {
    QFile data(imagePath);

    MITK_DEBUG << "Exportin in CSV " << imagePath.toStdString() << endl;

    if (!data.open(QFile::WriteOnly | QFile::Truncate))
        return;

    QTextStream output(&data);


    QString segmentationName = QString::fromStdString(node->GetName());
    MITK_DEBUG << "segmentationName: " << segmentationName.toStdString();
    //QModelIndex index = m_Controls.listWidget->currentIndex();
    //QString itemText = index.data(Qt::DisplayRole).toString();
    //MITK_INFO << itemText;
    QString header = "x,y,z";
    header = header + "," + segmentationName;
    //for(int row = 0; row < m_Controls.listWidget->count(); row++)
    // {
    //     QListWidgetItem *item = m_Controls.listWidget->item(row);
    //     header = header + "," + item->text();
    // }
    output << header << "\n";
    MITK_DEBUG << header.toStdString() << endl;
    // Now iterate through image and write down all voxel positions and image values
    // Here we use ITK to do this
    typedef itk::Image<unsigned char, 3> TSegmentationImage;
    typedef itk::Image<double, 3> TFeatureImage;
    typedef TFeatureImage::Pointer PTFeatureImage;



    mitk::Image* img = dynamic_cast<mitk::Image*>(node->GetData());
    // Create a new ITK image and assign the content of the MITK image to it
    TSegmentationImage::Pointer itkImg = TSegmentationImage::New();
    mitk::CastToItkImage(img,itkImg);

    std::vector<PTFeatureImage> featImgs;
    SetOfObjects::ConstPointer visibleParents = GetVisibleParentNodes(storage);
    SetOfObjects::ConstIterator iterator = visibleParents->Begin();
    DataNode::ConstPointer parent;
    while(iterator != visibleParents->End()) {


        parent = iterator->Value();


        MITK_DEBUG << "Parent: " << parent->GetName() << endl;
        //for(int row = 0; row < m_Controls.listWidget->count(); row++)
        //{

        //  QListWidgetItem *item = m_Controls.listWidget->item(row);
        if(storage->GetSources(parent)->Size() == 0) {
            mitk::Image* img = dynamic_cast<mitk::Image*>(parent->GetData());
            PTFeatureImage itkImg = TFeatureImage::New();
            mitk::CastToItkImage(img,itkImg);
            featImgs.push_back(itkImg);
        }
        iterator++;
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

bool STLExportService::exportToSTL(QString & path, const DataNode *dataNode, bool smoothing = false) {

    MITK_INFO << "Saving STL file from " << dataNode->GetName() << " at " << path.toStdString();

    if(smoothing) {
        MITK_INFO << "Smoothing" << endl;
        ExternalShapeSmoother smoother;
        smoother.smooth(dataNode);
        DataNode * smoothed = smoother.GetResult();
        if(smoother.Success()) {
            mitk::IOUtil::Save(smoothed->GetData(), path.toStdString());
        }
    }
    else {

        itk::SmartPointer<mitk::ImageToSurfaceFilter> i2sf = mitk::ImageToSurfaceFilter::New();
        itk::SmartPointer<mitk::Surface> surface = mitk::Surface::New();

        i2sf->SetInput(dynamic_cast<mitk::Image*> (dataNode->GetData()));
        i2sf->Update();
        surface = i2sf->GetOutput();
        mitk::DataNode::Pointer newNode = mitk::DataNode::New();
        newNode->SetData(surface);
        MITK_INFO << "Number of Point " <<surface->GetVtkPolyData()->GetNumberOfPoints();
        mitk::IOUtil::Save(surface, path.toStdString());
    }

    return true;
}

STLExportService::SetOfObjects::ConstPointer STLExportService::GetSelectedNodes(DataStorage *storage) {
    return storage->GetSubset(mitk::NodePredicateProperty::New("selected",mitk::BoolProperty::New(true)));
}

IsSelectedPredicate::IsSelectedPredicate()  {

}


STLExportService::SetOfObjects::ConstPointer STLExportService::GetVisibleParentNodes(DataStorage * storage) {

    return storage->GetSubset(mitk::NodePredicateProperty::New("visible",mitk::BoolProperty::New(true)));

}

bool IsSelectedPredicate::CheckNode(const mitk::DataNode *node) const {

    bool DEFAULT_VALUE = false;
    bool result = node->GetBoolProperty("selected",DEFAULT_VALUE);
    if(result == true) {
        STLExportService::Log("selected");
        STLExportService::printName(node);
    }
    return result;
}


