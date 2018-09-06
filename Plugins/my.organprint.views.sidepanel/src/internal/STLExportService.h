#ifndef STLEXPORTSERVICE_H
#define STLEXPORTSERVICE_H


#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <vector>
#include <mitkNodePredicateBase.h>
#include <QString>


class STLExportService

{
public:
    STLExportService();
    typedef mitk::DataNode DataNode;
    typedef mitk::DataStorage DataStorage;
    typedef mitk::DataStorage::SetOfObjects SetOfObjects;
    bool exportTo(QString & path, const DataNode * dataNode);
    int exportTo(QString & path, DataStorage * storage);
    void exportToCsv(QString & path, DataStorage * storage, const DataNode *);

    bool exportToSTL(QString & path, const DataNode * dataNode, bool smoothing);

    const DataNode * GetSelectedNode(DataStorage * storage);
    SetOfObjects::ConstPointer GetSelectedNodes(DataStorage * storage);
    void SetDataStorage(DataStorage * storage);

protected:



    DataStorage * m_DataStorage;

    const DataNode * GetParentNode(DataStorage * storage, const DataNode * node);

    SetOfObjects::ConstPointer GetNodesToExport(DataStorage * storage,const DataNode * selection);

    SetOfObjects::ConstPointer GetVisibleParentNodes(DataStorage * storage);

    bool isParent(DataStorage * storage, const DataNode * node);
public:



    static void printName(const DataNode *);

    static void Log(const char[]);
};

class IsSelectedPredicate : public mitk::NodePredicateBase {
public:
    IsSelectedPredicate();
    bool CheckNode(const mitk::DataNode * node) const override;
};

#endif // STLEXPORTSERVICE_H
