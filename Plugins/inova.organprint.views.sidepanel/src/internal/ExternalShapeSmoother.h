#ifndef EXTERNALSHAPESMOOTHER_H
#define EXTERNALSHAPESMOOTHER_H


#include <mitkDataNode.h>
#include <inova_organprint_views_sidepanel_Export.h>

class INOVA_ORGANPRINT_VIEWS_SIDEPANEL_EXPORT ExternalShapeSmoother
{

protected:

    mitk::DataNode * result;

    bool success = false;
public :



    bool Success();
    void smooth(const mitk::DataNode *node);
    mitk::DataNode * GetResult();


};

#endif // EXTERNALSHAPESMOOTHER_H
