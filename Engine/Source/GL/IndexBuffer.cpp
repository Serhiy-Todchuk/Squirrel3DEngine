// IndexBuffer.cpp: implementation of the IndexBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "IndexBuffer.h"
#include <Common/macros.h>

namespace Squirrel {

namespace GL {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
	
IndexBuffer::IndexBuffer(uint iIndNum, IndexSize indexSize):
	RenderData::IndexBuffer(iIndNum, indexSize), Buffer(GL_ELEMENT_ARRAY_BUFFER)
{
}

IndexBuffer::~IndexBuffer()
{
}

bool	IndexBuffer :: create ()
{
	int bufferSize = getIndexSize() * getIndicesNum();
	bool success = Buffer::create(bufferSize, getIndexBuff(), getStorageType() == RenderData::IBuffer::stGPUDynamicMemory);
	if(success)
	{
		//DELETE_ARR(mIndices);
	}
	return success;
}

bool IndexBuffer :: map(bool read, bool write)
{
	if(isCreated())
	{
		mCPUBuffer = mIndices;
		mIndices = reinterpret_cast<byte *>( mapBuffer(read, write) );
	}
	return mIndices != NULL;
}

void IndexBuffer :: unmap()
{
	if(isCreated())
	{
		unmapBuffer();
		mIndices = mCPUBuffer;
	}
}

void IndexBuffer :: update(int offset, int size)
{
	if(isCreated())
	{
		byte * data = &mIndices[offset];

		updateBuffer(offset, size, data);
	}
}

void IndexBuffer :: Unbind ()
{
	Buffer::Unbind(GL_ELEMENT_ARRAY_BUFFER);
}

} //namespace GL {

} //namespace Squirrel {
