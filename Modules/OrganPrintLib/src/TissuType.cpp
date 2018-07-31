#include "../include/TissuType.h"

std::string TissuType::PROPERTY_KEY = "Tissu type";

TissuType::TissuType(int id,TissuType::String * m_name):

    id(id),
    name(m_name)
{
    properties = new Properties(10);
}

TissuType::TissuType(int id,String name):
    id(id),name(nullptr)
{

    this->name = new String(name);

    properties = new Properties(10);

}

TissuType::TissuType(int id,const char * m_name):
    id(id)
{

    name = new String(m_name);
    properties = new Properties(10);
}

TissuType::TissuType(int id,String name,float arr[]):
    id(id)
    ,name(new String(name))
{


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

TissuType::String * TissuType::GetName() {
    return name;
}

TissuType * TissuType::pushProperty(float f) {

    this->properties->push_back(f);
    return this;

}

std::vector<float> * TissuType::GetWeightedVector(float weight) {
    Properties * props = new Properties(properties->size());
    CopyWeightedVector(weight,*props);
    return props;
}