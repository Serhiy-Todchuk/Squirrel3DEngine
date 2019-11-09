#include "XMLSerializer.h"
#include "Object.h"
#include "AtomicWrapper.h"
#include "XMLCommon.h"

namespace Squirrel {
namespace Reflection {
	
XMLSerializer::XMLSerializer()
{
	mCurrentNode = mDocument;
	mObjectIsOpened = true;
}

XMLSerializer::~XMLSerializer()
{
}

void XMLSerializer::writeToFile(const char * fileName)
{
	mDocument.save_file(fileName);
}
	
DATA_PTR XMLSerializer::getData(bool copy) const
{
	XMLDataWriter writer;
	mDocument.save(writer);
	return writer.getDataCopy();
}

void XMLSerializer::willWriteObjects(int objectsNum)
{

}

void XMLSerializer::writeObjectData(const char * data, size_t length)
{
	pugi::xml_node_type dataNodeType = pugi::node_pcdata;
	//TODO: check for pugi::node_cdata
	mCurrentNode.append_child(dataNodeType).set_value(data);
}

void XMLSerializer::writeAtomicValues(AtomicWrapper * atom)
{
	if(atom->getValuesNum() == 1)
	{
		std::string value = atom->getStringRepresentation(0);
		mCurrentNode.append_child(pugi::node_pcdata).set_value(value.c_str());
	}
	else
	{
		pugi::xml_attribute valuesNumAttr = mCurrentNode.append_attribute(XMLCommon::ValuesNumAttrName());
		valuesNumAttr.set_value( (uint)atom->getValuesNum() );

		for(int i = 0; i < atom->getValuesNum(); ++i)
		{
			pugi::xml_node valueNode = mCurrentNode.append_child(XMLCommon::AtomValueNodeName());
			std::string value = atom->getStringRepresentation(i);
			valueNode.append_child(pugi::node_pcdata).set_value(value.c_str());
		}
	}
}

void XMLSerializer::beginWriteObject(const std::string& className, const std::string& name)
{
	if(mObjectIsOpened)
	{
		mCurrentNode = mCurrentNode.append_child(XMLCommon::ObjectElementName());
	}
	else
	{
		mCurrentNode = mCurrentNode.parent().append_child(XMLCommon::ObjectElementName());
	}
	mObjectIsOpened = true;

	pugi::xml_attribute nameAttr = mCurrentNode.append_attribute(XMLCommon::NameAttrName());
	nameAttr.set_value(name.c_str());

	pugi::xml_attribute classAttr = mCurrentNode.append_attribute(XMLCommon::ClassAttrName());
	classAttr.set_value(className.c_str());
}


void XMLSerializer::endWriteObject()
{
	if(!mObjectIsOpened)
	{
		if(!mCurrentNode.parent().empty())
		{
			mCurrentNode = mCurrentNode.parent();
		}
	}
	mObjectIsOpened = false;
}

}//namespace Reflection {
}//namespace Squirrel