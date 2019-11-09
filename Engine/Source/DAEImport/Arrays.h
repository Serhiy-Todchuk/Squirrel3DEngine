#pragma once

#include "macros.h"
#include <Common/common.h>

namespace Squirrel {
namespace DAEImport { 

template <class TElem>
class DataArray
{
	inline bool isSpace(const char& c)
	{
		return (c == ' ' || c == '\r' || c == '\n' || c == '\t');
	}

	inline void initWithData(int size, const char * src)
	{
		mArray = NULL;

		if(size <= 0) return;

		mArray = new TElem[mSize = size];

		//tmp buffer for word
		char tmpStr[128];

		size_t strLen = strlen(src);
		int strStart = 0;//word start
		int strEnd = 0;//word end
		int strPos = 0;//position in src string
		int counter = 0;//counter of words

		//go through string and parse words
		while(strPos < strLen)
		{
			//skip spaces before word
			for(;;++strPos)
			{
				if(strPos == strLen || !isSpace(src[strPos]))
				{
					strStart = strPos;
					break;
				}
			}

			//find the end of word
			for(strPos = strStart;; ++strPos)
			{
				if(strPos == strLen || isSpace(src[strPos]))
				{
					strEnd = strPos - 1;
					break;
				}
			}

			//extract word from src string
			int tmpStrLen = strEnd - strStart + 1;
			memcpy(tmpStr, &src[strStart], tmpStrLen);
			tmpStr[tmpStrLen] = '\0';

			//convert word to element
			mArray[counter] = elemFromString(tmpStr);

			//skip one space
			//strStart = strPos + 1;

			//inc counter of words
			if(++counter == mSize)	break;
		}
	}

public:
	inline DataArray()
	{
		mArray = NULL;
		mSize = 0;
	}
	inline DataArray(int size)
	{
		mArray = NULL;

		if(size <= 0) return;

		mArray = new TElem[mSize = size];
	}

	inline DataArray(const char * src)
	{
		//calculate number of spaces to determine size of array
		size_t strLen = strlen(src);
		int wordsNum = 0;
		bool word = false;
		for(int i = 0; i < strLen; ++i)
		{
			if(!isSpace(src[i]) && !word)
			{
				word = true;
				++wordsNum;
			}
			if(isSpace(src[i]) && word)
			{
				word = false;
			}
		}
		//actual size of array is bigger than number of spaces by one
		int size = wordsNum;

		initWithData(size, src);
	}

	inline DataArray(int size, const char * src)
	{
		initWithData(size, src);
	}

	inline DataArray(const DataArray<TElem>& copyMe)
	{
		mArray = new TElem[mSize = copyMe.mSize];
		for(int i = 0; i < mSize; ++i)
		{
			mArray[i] = copyMe.mArray[i];
		}
	}

	inline ~DataArray()
	{
		if(mArray != NULL)
			delete[]mArray;
	}

	inline int getSize() const
	{ 
		return mSize; 
	}

	inline TElem * getArray()
	{
		return mArray;
	}

	inline const TElem * getArray() const
	{
		return mArray;
	}

	inline const TElem& getElement(int index) const
	{
		return mArray[index];
	}

protected:

	inline TElem elemFromString(char * src);

	int mSize;
	TElem * mArray;
};

template <>
inline std::string DataArray<std::string>::elemFromString(char * src)
{
	return std::string(src);
}

template <>
inline float DataArray<float>::elemFromString(char * src)
{
	size_t srclen = strlen(src);
	for(size_t i = 0; i < srclen; ++i)
		if(src[i] == ',') src[i] = '.';
	return static_cast<float>(atof(src));
}

template <>
inline int DataArray<int>::elemFromString(char * src)
{
	return atoi(src);
}

template <class TElem>
class TupleArray: public DataArray<TElem>
{
public:

	TupleArray(): DataArray<TElem>() { mCount = 0; mStride = 0; }
	TupleArray(int size): DataArray<TElem>(size) { mCount = size; mStride = 1; }
	TupleArray(int size, const char * src): DataArray<TElem>(size, src) { mCount = size; mStride = 1; }
	TupleArray(const char * src): DataArray<TElem>(src) { mCount = DataArray<TElem>::mSize; mStride = 1; }
	TupleArray(const TupleArray<TElem>& copyMe): DataArray<TElem>(copyMe) { mCount = copyMe.mCount; mStride = copyMe.mStride; }

	inline void setAccessorInfo(int count, int stride)
	{
		mCount = count; mStride = stride;
	}

	inline int getCount() const
	{ 
		return mCount; 
	}

	inline int getStride() const
	{ 
		return mStride; 
	}

	inline TElem * getTuple(int index)
	{
		return &this->mArray[index * mStride];
	}

private:

	int mCount;
	int mStride;
};

typedef TupleArray<float>		FloatArray;
typedef TupleArray<int>			IntArray;
typedef DataArray<std::string>	NameArray;

}//namespace DAEImport { 
}//namespace Squirrel {