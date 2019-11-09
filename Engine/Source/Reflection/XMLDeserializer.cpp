#include "XMLDeserializer.h"
#include "Object.h"
#include "AtomicWrapper.h"
#include "XMLCommon.h"

namespace Squirrel {
namespace Reflection {

XMLDeserializer::XMLDeserializer()
{
}

XMLDeserializer::XMLDeserializer(const void * srcBuffer, size_t bufferSize)
{
	loadFrom(srcBuffer, bufferSize);
}

XMLDeserializer::~XMLDeserializer()
{
}

void XMLDeserializer::loadFrom(const void * srcBuffer, size_t bufferSize, bool copy)
{
	mData.reset(new RawData((char *)srcBuffer, bufferSize, copy));
	
	//user responsible for deallocating buffer himself
	mDocument.load_buffer(mData->data, mData->length);

	mCurrentNode = pugi::xml_node();
	mObjectIsOpened = false;
}

void XMLDeserializer::loadFrom(const char * srcFileName)
{
	mDocument.load_file(srcFileName);

	mCurrentNode = pugi::xml_node();
	mObjectIsOpened = false;
}

int XMLDeserializer::getObjectsNum()
{
	int counter = 0;

	pugi::xml_node node = (mCurrentNode.empty()) ? mDocument.child(XMLCommon::ObjectElementName()) :
		(mObjectIsOpened ? mCurrentNode.first_child() : mCurrentNode.parent().first_child());
	while(!node.empty())
	{
		++counter;
		node = node.next_sibling();
	}

	return counter;
}

DATA_PTR XMLDeserializer::readObjectData()
{
	const char* data = mCurrentNode.child_value();
	return DATA_PTR(new RawData((char *)data, strlen(data), true));
}
	
void XMLDeserializer::readAtomicValues(AtomicWrapper * atom)
{
	int valuesNum = 0;

	pugi::xml_attribute valuesNumAttr = mCurrentNode.attribute(XMLCommon::ValuesNumAttrName());
	if(!valuesNumAttr.empty())
	{
		valuesNum = atoi(valuesNumAttr.value());
		if(valuesNum > atom->getValuesNum()) valuesNum = atom->getValuesNum();
	}
	else
	{
		valuesNum = 1;
	}

	if(valuesNum == 1)
	{
		std::string value = mCurrentNode.child_value();
		atom->parseString(value, 0);
	}
	else
	{
		pugi::xml_node valueNode = mCurrentNode.child(XMLCommon::AtomValueNodeName());
		for(int i = 0; i < valuesNum; ++i)
		{
			std::string value = valueNode.child_value();
			atom->parseString(value, i);

			valueNode = valueNode.next_sibling();
			if(!valueNode)
				break;
		}
	}
}

bool XMLDeserializer::beginReadObject(std::string& className, std::string& name)
{
	mCurrentNode = (mCurrentNode.empty()) ? mDocument.child(XMLCommon::ObjectElementName()) :
		(mObjectIsOpened ? mCurrentNode.first_child() : mCurrentNode.next_sibling());
	mObjectIsOpened = true;

	if(mCurrentNode.empty()) return false;

	pugi::xml_attribute nameAttr = mCurrentNode.attribute(XMLCommon::NameAttrName());
	name = nameAttr.value();

	pugi::xml_attribute classAttr = mCurrentNode.attribute(XMLCommon::ClassAttrName());
	className = classAttr.value();
	
	return true;
}

void XMLDeserializer::endReadObject()
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