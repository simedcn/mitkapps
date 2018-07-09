#include "STLExportService.h"
#include <mitkSurface.h>
#include <mitkImageToSurfaceFilter.h>
#include <QTextStream>
#include <mitkIOUtil.h>
#include <mitkNodePredicateProperty.h>
#include <QFile>

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

    mitk::IOUtil::Save(surface, sstr.str());

    return true;

}

int STLExportService::exportTo(QString & path, DataStorage * storage) {

    cout << "Exporting to " << path.toStdString() << endl;

    const DataNode * selection = GetSelectedNode(storage);

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
    cout << "Node [" << &node << "] " << node->GetName() << endl;
}
void STLExportService::Log(const char msg[]) {
    cout << "[STLExportService] " << msg << endl;
}

IsSelectedPredicate::IsSelectedPredicate()  {

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
