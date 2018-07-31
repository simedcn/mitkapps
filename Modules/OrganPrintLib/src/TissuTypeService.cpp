#include "../include/TissuTypeService.h"


using namespace std;

TissuTypeService * TissuTypeService::s_instance = nullptr;

TissuTypeService::TissuTypeService()
{


    tissuTypeList = new TissuTypeList(10);
    cout << "Created the list" <<endl;
    TissuType * tissuType;

    tissuType= new TissuType(0,"Gray matter");
    tissuType->pushProperty(0.2);
    tissuType->pushProperty(0.3);
    tissuTypeList->push_back(tissuType);
    cout << "Added the first tissu" << endl;
    tissuType = new TissuType(1,"Bone");
    tissuType->pushProperty(10);
    tissuType->pushProperty(20);
    tissuTypeList->push_back(tissuType);


}
TissuTypeService::TissuTypeList * TissuTypeService::GetTissuTypeList() {
    return tissuTypeList;
}


TissuTypeService * TissuTypeService::GetInstance() {

    if(s_instance == nullptr) {
        s_instance = new TissuTypeService();
    }
    return s_instance;
}


