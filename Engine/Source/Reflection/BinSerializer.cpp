#include "BinSerializer.h"
#include "Object.h"
#include "AtomicWrapper.h"

namespace Squirrel {
namespace Reflection {
	
BinSerializer::BinSerializer()
{
}

BinSerializer::~BinSerializer()
{
}
	
DATA_PTR BinSerializer::getData(bool copy) const
{
	if(copy)
		return mDataWriter.getDataCopy();

	return mDataWriter.getData();
}

void BinSerializer::willWriteObjects(int objectsNum)
{
	uint32 objsNum = static_cast<uint32>(objectsNum);
	mDataWriter.write(&objsNum, sizeof(objsNum));
}

void BinSerializer::writeAtomicValues(AtomicWrapper * atom)
{
	uint16 valuesNum = static_cast<uint16>(atom->getValuesNum());
	mDataWriter.write(&valuesNum, sizeof(valuesNum));
	
	for(uint16 i = 0; i < valuesNum; ++i)
	{
		mDataWriter.write(atom->getBinaryBuffer(i), atom->getBinaryBufferSize(i));
	}
}

void BinSerializer::beginWriteObject(const std::string& className, const std::string& name)
{
	mDataWriter.write(className.c_str(), className.length() + 1);
	mDataWriter.write(name.c_str(), name.length() + 1);
}


void BinSerializer::endWriteObject()
{
}

}//namespace Reflection {
}//namespace Squirrel