#ifndef TISSUTYPE_H
#define TISSUTYPE_H

#include <iostream>
#include <vector>
#include "OrganPrintLibExports.h"

class OrganPrintLib_EXPORT TissuType
{



public:

    typedef std::vector<float> Properties;
    typedef std::string String;


    TissuType(int id,String * name);

    TissuType(int id,String name);

    TissuType(int id,String name, float arr[]);

    TissuType(int id,const char *);

    String * GetName();

    void SetProperties(Properties *  properties);
    TissuType * pushProperty(float f);

    void CopyWeightedVector(float weight, Properties & outputVector);
    Properties * GetWeightedVector(float weight);

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4251)
#endif
    static std::string PROPERTY_KEY;

private:
    int id;
    String * name;
    Properties * properties;
};

#endif // TISSUTYPE_H
