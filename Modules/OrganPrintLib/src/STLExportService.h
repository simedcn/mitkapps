#ifndef STLEXPORTSERVICE_H
#define STLEXPORTSERVICE_H

#include <QString>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <vector>
#include <mitkNodePredicateBase.h>

class STLExportService
{
public:
    STLExportService();
    typedef mitk::DataNode DataNode;
    typedef mitk::DataStorage DataStorage;
    typedef mitk::DataStorage::SetOfObjects SetOfObjects;

    bool export(QString & path, DataNode* dataNode);

    int export(QString & path, DataStorage * storage);

protected:

    DataNode * GetSelectedNode(DataStorage * storage);

    DataNode * GetParentNode(DataStorage * storage, DataNode * node);

    vector<DataNode*> GetNodesToExport(DataStorage * storage);

    bool isParent(DataStorage * storage, DataNode * node);



};

class IsSelectedPredicate : mitk::NodePredicateBase {
    virtual CheckNode(DataNode * node);
}

#endif // STLEXPORTSERVICE_H
