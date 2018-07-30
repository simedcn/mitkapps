#ifndef TISSUTYPE_H
#define TISSUTYPE_H

#include <iostream>
#include <vector>

class TissuType
{



public:

    typedef std::vector<float> Properties;
    typedef std::string String;


    TissuType(String & name);

    TissuType(String & name, float arr[]);

    void SetProperties(Properties *  properties);


    void CopyWeightedVector(float weight, Properties & outputVector);
    Properties * GetWeightedVector(float weight);



private:
    String & name;
    Properties * properties;
};

#endif // TISSUTYPE_H
