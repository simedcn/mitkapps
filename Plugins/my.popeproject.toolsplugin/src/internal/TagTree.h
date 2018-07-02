#ifndef TagTree_H
#define TagTree_H

#include <mitkBaseProperty.h>

#include <itkSmartPointer.h>

#include <memory>
#include <list>
#include <map>

using namespace std;

struct Tag
{
public:
	Tag(const string& name, const string& value);
public:
	string name;
	string value;
};

using PropertyMap = map<string, itk::SmartPointer<mitk::BaseProperty>>;

struct TagTree;
using TagNode = shared_ptr<TagTree>;

struct TagTree
{
public:
	map<string, TagNode> tagGroups;
	list<shared_ptr<Tag>> tags;

public:
	static shared_ptr<TagTree> Create(const PropertyMap& property_map);
};


#endif // TagTree_H