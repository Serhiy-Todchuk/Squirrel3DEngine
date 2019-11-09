#pragma once

#include "RawData.h"
#include <string>

namespace Squirrel {
namespace Reflection {

class AtomicWrapper;
	
class SQREFLECTION_API Serializer
{	
public://ctor/dtor

	Serializer() {}
	virtual ~Serializer() {}

public://methods

	virtual DATA_PTR getData(bool copy = true) const = 0;

	virtual void willWriteObjects(int objectsNum) = 0;
	//TODO: to find out another way to store data of atomic object. 
	//The main problem is in that different serializers 
	//need different format of data to store (binary, text...).
	//Maybe need to make AtomicWrapper not inherited from Object?
	virtual void writeAtomicValues(AtomicWrapper * atom) = 0;
	virtual void writeObjectData(const char * data, size_t length) = 0;
	virtual void beginWriteObject(const std::string& className, const std::string& name) = 0;
	virtual void endWriteObject() = 0;
};

}//namespace Reflection {
}//namespace Squirrel