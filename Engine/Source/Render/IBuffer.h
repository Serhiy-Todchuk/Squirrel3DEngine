// IndexBuffer.h: interface for the IndexBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "IContextObject.h"
#include "Utils.h"
#include "common/types.h"
#include "macros.h"

namespace Squirrel {

namespace RenderData {

class SQRENDER_API IBuffer
{
public:

	enum StorageType
	{
		stCPUMemory = 0,
		stGPUDynamicMemory = 1,
		stGPUStaticMemory = 2
	};

public:
	IBuffer() : mStorageType(stGPUStaticMemory), mCPUBuffer(NULL) {};
	virtual ~IBuffer () {};
	
	virtual bool map(bool read, bool write) = 0;
	virtual void unmap() = 0;

	virtual void update(int offset, int size) = 0;

	inline StorageType	getStorageType()				{ return mStorageType; }
	inline void			setStorageType(StorageType st)	{ mStorageType = st; }

protected:

	StorageType	mStorageType;

	byte* mCPUBuffer;

};


} //namespace RenderData {

} //namespace Squirrel {
