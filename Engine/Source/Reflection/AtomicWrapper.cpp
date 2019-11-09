#include "AtomicWrapper.h"

namespace Squirrel {
namespace Reflection {

AtomicWrapper::AtomicWrapper()
{
	mClassNamesStack.push_back( "AtomicWrapper" );

	mValues		= NULL;
	mValuesNum	= 0;
	mOwner		= true;
	mForceBinary= false;

	mServingTypeName	= "";
	mServingType		= tUnknown;
}

AtomicWrapper::~AtomicWrapper()
{
	if(mValues != NULL && mOwner)
	{
		delete[]mValues;
	}
}

bool AtomicWrapper::sameValues(const AtomicWrapper* otherAtom) const
{
	if(mValuesNum != otherAtom->mValuesNum)
		return false;

	if(mServingType == tString)
	{
		for(int i = 0; i < mValuesNum; ++i)
		{
			if(static_cast<std::string *>(mValues)[i] !=
			   static_cast<std::string *>(otherAtom->mValues)[i])
				return false;
		}
	}
	else
	{
		if(memcmp(mValues, otherAtom->mValues, mValuesNum * mServingTypeSize) != 0)
			return false;
	}

	return true;
}

void* AtomicWrapper::getBinaryBuffer(size_t index) const
{
	if(mServingType == tString)
	{
		const TChar * stringBuffer = static_cast<std::string *>(mValues)[index].c_str();
		return const_cast<void*>( static_cast<const void *>(stringBuffer) );
	}
	return getValuePointer(index);
}

void AtomicWrapper::setBinaryBufferData(void* data, size_t index)
{
	if(mServingType == tString)
	{
		//for string support only first value (considering that static arrays of string are rare) 
		std::string& str = static_cast<std::string *>(mValues)[index];
		TChar* newStrData = static_cast<TChar*>(data);
		str = newStrData;
		return;
	}
	memcpy( getValuePointer(index), data, getBinaryBufferSize(index) );
}

size_t AtomicWrapper::getBinaryBufferSize(size_t index) const
{
	if(mServingType == tString)
	{
		size_t stdLen = static_cast<std::string *>(mValues)[index].length() + 1;
		return stdLen * sizeof(std::string::value_type);
	}
	return mServingTypeSize;
}

void* AtomicWrapper::getValuePointer(size_t index) const
{
	ASSERT( index < mValuesNum );
	char * buff = static_cast<char *>(mValues);
	size_t offset = mServingTypeSize * index;
	return static_cast<void *>(&buff[offset]);
}

void AtomicWrapper::serialize(Serializer * serializer)
{
	serializer->writeAtomicValues( this );
}

void AtomicWrapper::deserialize(Deserializer * deserializer)
{
	deserializer->readAtomicValues( this );
}

}//namespace Reflection {
}//namespace Squirrel