#pragma once

#include "RawData.h"
#include "macros.h"
#include <string>

namespace Squirrel {
namespace Reflection {

class AtomicWrapper;

class SQREFLECTION_API Deserializer
{
public://ctor/dtor

	Deserializer() {}
	virtual ~Deserializer() {}

public://methods

	virtual void loadFrom(const void * srcBuffer, size_t bufferSize, bool copy = true) = 0;

	virtual int getObjectsNum() = 0;
	virtual void readAtomicValues(AtomicWrapper * atom) = 0;
	virtual DATA_PTR readObjectData() = 0;
	virtual bool beginReadObject(std::string& className, std::string& name) = 0;
	virtual void endReadObject() = 0;
};

}//namespace Reflection {
}//namespace Squirrel