#include "BinDeserializer.h"
#include "Object.h"
#include "AtomicWrapper.h"

namespace Squirrel {
namespace Reflection {

BinDeserializer::BinDeserializer()
{
	mPos = 0;
}

BinDeserializer::BinDeserializer(const void * srcBuffer, size_t bufferSize)
{
	mPos = 0;
	loadFrom(srcBuffer, bufferSize);
}

BinDeserializer::~BinDeserializer()
{
}

void BinDeserializer::loadFrom(const void * srcBuffer, size_t bufferSize, bool copy)
{
	mData.reset(new RawData((char *)srcBuffer, bufferSize, copy));
}

int BinDeserializer::getObjectsNum()
{
	return read<uint32>();
}

void BinDeserializer::readAtomicValues(AtomicWrapper * atom)
{
	uint16 valuesNum = read<uint16>();
	
	ASSERT(valuesNum == atom->getValuesNum());
	
	if(atom->getValuesNum() < valuesNum)
		valuesNum = atom->getValuesNum();
	
	for(uint16 i = 0; i < valuesNum; ++i)
	{
		atom->setBinaryBufferData(&mData->data[mPos], i);
		mPos += atom->getBinaryBufferSize(i);
	}
}

bool BinDeserializer::beginReadObject(std::string& className, std::string& name)
{
	className = &mData->data[mPos];
	mPos += className.length() + 1;
	
	name = &mData->data[mPos];
	mPos += name.length() + 1;

	return true;
}

void BinDeserializer::endReadObject()
{
}

}//namespace Reflection {
}//namespace Squirrel