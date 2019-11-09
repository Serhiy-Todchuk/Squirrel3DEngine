// VertexBuffer.cpp: implementation of the VertexBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "VertexBuffer.h"
#include <Common/macros.h>

namespace Squirrel {

namespace GL {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VertexBuffer::VertexBuffer(int vertType, int vertsNum):
	RenderData::VertexBuffer(vertType, vertsNum, NULL), Buffer(GL_ARRAY_BUFFER)
{
}

VertexBuffer::~VertexBuffer()
{
}

bool	VertexBuffer :: create ()
{
	size_t bufferSize = getVertexSize() * getVertsNum();
	bool success = Buffer::create(bufferSize, getVerts(), getStorageType() == RenderData::VertexBuffer::stGPUDynamicMemory);
	if(success)
	{
		//DELETE_ARR(mVerts);
	}
	return success;
}

bool VertexBuffer :: map(bool read, bool write)
{
	if(isCreated())
	{
		mCPUBuffer = mVerts;
		void * buffer = mapBuffer(read, write);
		mVerts = reinterpret_cast<byte *>( buffer );
	}
	return mVerts != NULL;
}

void VertexBuffer :: unmap()
{
	if(isCreated())
	{
		unmapBuffer();
		mVerts = mCPUBuffer;
	}
}

void VertexBuffer :: update(int offset, int size)
{
	if(isCreated())
	{
		byte * data = &mVerts[offset];

		updateBuffer(offset, size, data);
	}
}

void VertexBuffer :: Unbind ()
{
	Buffer::Unbind(GL_ARRAY_BUFFER);
}

} //namespace GL {

} //namespace Squirrel {
