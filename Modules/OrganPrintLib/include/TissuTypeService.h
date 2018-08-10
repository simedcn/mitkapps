#ifndef TISSUTYPESERVICE_H
#define TISSUTYPESERVICE_H

#include <vector>
#include <TissuType.h>
#include "OrganPrintLibExports.h"

class OrganPrintLib_EXPORT TissuTypeService
{
public:
    TissuTypeService();

    typedef std::vector<TissuType*> TissuTypeList;

    TissuTypeList * GetTissuTypeList();

protected:
    TissuTypeList * tissuTypeList;


public:
    //Static declarations
    static TissuTypeService * GetInstance();

private:
    static TissuTypeService * s_instance;



};

#endif // TISSUTYPESERVICE_H
