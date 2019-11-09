#pragma once

#include "Deserializer.h"
#include "RawData.h"

namespace Squirrel {
namespace Reflection {

class SQREFLECTION_API BinDeserializer:	public Deserializer
{
	std::auto_ptr<RawData> mData;
	size_t mPos;
	
public://ctor/dtor

	BinDeserializer();
	BinDeserializer(const void * srcBuffer, size_t bufferSize);
	virtual ~BinDeserializer();

public://methods

	virtual void loadFrom(const void * srcBuffer, size_t bufferSize, bool copy = true);

	virtual int getObjectsNum();
	virtual void readAtomicValues(AtomicWrapper * atom);
	virtual DATA_PTR readObjectData() { return DATA_PTR(NULL); } //TODO: implement
	virtual bool beginReadObject(std::string& className, std::string& name);
	virtual void endReadObject();
	
private://methods
	
	template <class _T>
	_T read()
	{
		size_t typeSize = sizeof(_T);
		
		_T var;
		
		memcpy(&var, &mData->data[mPos], typeSize);
		
		mPos += typeSize;
		
		return var;
	}
};

}//namespace Reflection {
}//namespace Squirrel