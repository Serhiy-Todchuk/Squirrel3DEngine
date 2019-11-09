#pragma once

#include "types.h"
#include "macros.h"

template <class T> 
class BufferArray
{
public:
	BufferArray()
	{
		mCount	= 0;
		mValues	= NULL;
	}
	~BufferArray()
	{
		DELETE_ARR( mValues );
	}
	//copy ctor
	BufferArray(const BufferArray& srcBuffer)
	{
		copy((*this), srcBuffer);
	}

	inline void operator =(const BufferArray& srcBuffer)
	{
		copy((*this), srcBuffer);
	}

	static void copy(BufferArray& dstBuffer, const BufferArray& srcBuffer)
	{
		dstBuffer.mCount	= 0;
		dstBuffer.mValues	= NULL;
		dstBuffer.setCount(srcBuffer.mCount);
		for(int i = 0; i < dstBuffer.mCount; ++i)
		{
			const T& srcValue = srcBuffer.mValues [ i ];
			dstBuffer.mValues[ i ] = srcValue;
		}
	}

	void setCount(int newCount)
	{
		if(mCount == newCount)
		{
			return;
		}
		DELETE_ARR(mValues);
		if((mCount = newCount)>0)
		{
			mValues = new T[mCount];
		}
	}
	int getCount()
	{
		return mCount;
	}
	T * getData()
	{
		return mValues;
	}

private:
	int mCount;
	T * mValues;
};
