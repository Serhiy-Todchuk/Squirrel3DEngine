#pragma once

#include "Deserializer.h"
#include "RawData.h"
#include <pugixml/pugixml.hpp>

namespace Squirrel {
namespace Reflection {

class SQREFLECTION_API XMLDeserializer:	public Deserializer
{
	pugi::xml_document mDocument;
	pugi::xml_node mCurrentNode;
	bool mObjectIsOpened;
	std::auto_ptr<RawData> mData;
	
public://ctor/dtor
	
	XMLDeserializer();
	XMLDeserializer(const void * srcBuffer, size_t bufferSize);
	virtual ~XMLDeserializer();
	
public://methods
	
	virtual void loadFrom(const void * srcBuffer, size_t bufferSize, bool copy = true);
	void loadFrom(const char * srcFileName);
	
	virtual int getObjectsNum();
	virtual void readAtomicValues(AtomicWrapper * atom);
	virtual DATA_PTR readObjectData();
	virtual bool beginReadObject(std::string& className, std::string& name);
	virtual void endReadObject();
};

}//namespace Reflection {
}//namespace Squirrel