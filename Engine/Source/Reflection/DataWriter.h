#pragma once

#include "RawData.h"

namespace Squirrel {
namespace Reflection {

class DataWriter
{
	size_t mMemIncrement;
	char * mData;
	size_t mLength;
	size_t mCapacity;
	
public:
	DataWriter():mMemIncrement(1024), mData(NULL), mLength(0), mCapacity(0) {
		mCapacity = mMemIncrement;
		mData = new char[mCapacity];}
	virtual ~DataWriter() {
		if(mData != NULL)
			delete[]mData;
	}
	
	void write(const void* data, size_t size)
	{
		size_t newSize = mLength + size;
		
		if( newSize > mCapacity )
		{
			size_t bytesNeed = newSize - mCapacity;
			size_t newCapacity = mCapacity + bytesNeed;
			if(bytesNeed < mMemIncrement)
			{
				newCapacity = mCapacity + mMemIncrement;
			}
			char * newData = new char[mCapacity = newCapacity];
			if(mLength > 0)
			{
				memcpy( newData, mData, mLength );
			}
			delete[]mData;
			mData = newData;
		}
		
		memcpy(mData + mLength, data, size);
		mLength = newSize;
	}
	
	DATA_PTR getData() const {
		return DATA_PTR(new RawData(mData, mLength));
	}
	DATA_PTR getDataCopy() const {
		return DATA_PTR(new RawData(mData, mLength, true));
	}
};

}//namespace Reflection {
}//namespace Squirrel