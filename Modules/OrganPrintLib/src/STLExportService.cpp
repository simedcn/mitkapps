#include "STLExportService.h"
#include <mitkSurface.h>
#include <mitkImageToSurfaceFilter.h>
#include <QTextStream>
#include <mitkIOUtil.h>

STLExportService::STLExportService()
{

}

bool STLExportService::export(QString & path, DataNode*dataNode) {

    MITK_INFO << "Export STLs";
    itk::SmartPointer<mitk::ImageToSurfaceFilter> i2sf = mitk::ImageToSurfaceFilter::New();
    itk::SmartPointer<mitk::Surface> surface = mitk::Surface::New();

    std::string name;
    DataNode*node_selected = dataNode;
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

    return true;

}

int STLExportService::export(QString & path, DataStorage * storage) {

    vector<DataNode*> exportList = GetNodesToExport(storage);

    if(!exportList) {
        return false;
    }

    for(DataNode*dataNode : exportList) {

        export(path,dataNode);
    }
    return (int) exportList.size();

}
DataNode * STLExportService::GetSelectedNode(DataStorage*) {
    const IsSelectedPredicate isSelected;
    return storage->GetNode(isSelected);
}

DataNode * STLExportService::GetParentNode(DataStorage *storage, DataNode *node) {

    SetOfObjects sources = storage->GetSource(node);

    if(sources.size() ==0) {
        return nullptr;

    }
    else {
        return sources[0];
    }
}
vector<DataNode*> STLExportService::GetNodesToExport(DataStorage *storage) {

    vector<DataNode*> empty(0);

    DataNode * selection = GetSelectedNode(storage);

    if(selection == nullptr) {
        return empty;
    }

}
bool STLExportService::isParent(DataStorage * storage, DataNode*node) {
    return GetParentNode(storage,node) == nullptr;
}



bool IsSelectedPredicate::CheckNode(DataNode *node) {
    return node->GetBoolProperty("selected"));
}
