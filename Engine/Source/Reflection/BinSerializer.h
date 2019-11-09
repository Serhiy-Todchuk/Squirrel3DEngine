#pragma once

#include "Serializer.h"
#include "DataWriter.h"

namespace Squirrel {
namespace Reflection {
	
class SQREFLECTION_API BinSerializer:
	public Serializer
{
	DataWriter mDataWriter;
	
public://ctor/dtor

	BinSerializer();
	virtual ~BinSerializer();

public://methods

	virtual DATA_PTR getData(bool copy = true) const;

	virtual void willWriteObjects(int objectsNum);
	virtual void writeAtomicValues(AtomicWrapper * atom);
	virtual void writeObjectData(const char * data, size_t length) {} //TODO: impelement
	virtual void beginWriteObject(const std::string& className, const std::string& name);
	virtual void endWriteObject();
};

}//namespace Reflection {
}//namespace Squirrel