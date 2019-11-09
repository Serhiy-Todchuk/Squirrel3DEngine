#pragma once

#include "Serializer.h"
#include "DataWriter.h"
#include <pugixml/pugixml.hpp>

namespace Squirrel {
namespace Reflection {
	
class XMLDataWriter: public pugi::xml_writer, public DataWriter
{
public:
	XMLDataWriter() {}
	~XMLDataWriter() {}
	
	virtual void write(const void* data, size_t size)
	{
		DataWriter::write(data, size);
	}
};

class SQREFLECTION_API XMLSerializer: 
	public Serializer
{
	pugi::xml_document	mDocument;
	pugi::xml_node		mCurrentNode;
	bool				mObjectIsOpened;
	
public://ctor/dtor

	XMLSerializer();
	virtual ~XMLSerializer();

public://methods

	virtual DATA_PTR getData(bool copy = true) const;

	void writeToFile(const char * fileName);

	virtual void willWriteObjects(int objectsNum);
	virtual void writeAtomicValues(AtomicWrapper * atom);
	virtual void writeObjectData(const char * data, size_t length);
	virtual void beginWriteObject(const std::string& className, const std::string& name);
	virtual void endWriteObject();
};

}//namespace Reflection {
}//namespace Squirrel