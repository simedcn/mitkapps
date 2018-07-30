#include "../include/TissuType.h"

TissuType::TissuType(std::string & m_name):
    name(m_name)
{

}

TissuType::TissuType(String &name,float arr[]):  name(name) {


    uint size = sizeof(*arr);


    Properties * props = new Properties(size);

    for(uint i = 0; i!=size; i++) {
        (*props)[i] = arr[i];
    }

    this->properties = props;
}

void TissuType::SetProperties(TissuType::Properties  * properties) {
    this->properties = properties;
}
void TissuType::CopyWeightedVector(float weight, TissuType::Properties & outputVector) {
    uint size = std::min<uint>(outputVector.size(),properties->size());
    for(uint i = 0; i!= size; i++) {
        float p = properties->at(i);
        outputVector[i] = p * weight;
    }
}

std::vector<float> * TissuType::GetWeightedVector(float weight) {
    Properties * props = new Properties(properties->size());
    CopyWeightedVector(weight,*props);
    return props;
}
