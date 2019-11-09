// Buffer.cpp: implementation of the Buffer class.
//
//////////////////////////////////////////////////////////////////////

#include "Buffer.h"
#include "common/macros.h"
#include "Utils.h"
#include <map>

namespace Squirrel {

namespace GL {

std::map<uint, Buffer *> sBoundBuffers;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Buffer::Buffer(uint bufferType): mBufferType(bufferType)
{
	mBufferId = 0;
	mCreated = false;
	mMapped = NULL;
}

Buffer::~Buffer()
{
	setPool(NULL);
}

void Buffer :: generate()
{
}

void Buffer :: destroy()
{
	if(mBufferId != 0)
	{
		std::map<uint, Buffer *>::iterator it = sBoundBuffers.find(mBufferType);

		if(it != sBoundBuffers.end())
		{
			sBoundBuffers.erase(it);
		}

		glDeleteBuffers(1, &mBufferId);
		mBufferId = 0;
	}
}

bool	Buffer :: create (uint bufferSize, void * bufferData, bool dynamic)
{
	bool success = true;
	glGenBuffers(1, &mBufferId);
	mCreated = true;

	bind();

	glBufferData(mBufferType, bufferSize, bufferData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);//GL_DYNAMIC_DRAW

	//check buffer
	int bufferSizeTest = 0;
	glGetBufferParameteriv(mBufferType, GL_BUFFER_SIZE, &bufferSizeTest);
	if( bufferSizeTest < (int)bufferSize )
		success = false;

	//unbind();

	return success;
}

void Buffer::bind ()
{
	std::map<uint, Buffer *>::iterator it = sBoundBuffers.find(mBufferType);

	if(it != sBoundBuffers.end())
	{
		if(it->second == this)
		{
			return;
		}
	}

	sBoundBuffers[mBufferType] = this;

	glBindBuffer(mBufferType, mBufferId);
}

void Buffer :: unbind ()
{
	Unbind(mBufferType);
}

void* Buffer :: mapBuffer(bool read, bool write)
{
	ASSERT(read || write);
	uint access = read ? (write ? GL_READ_WRITE : GL_READ_ONLY) : GL_WRITE_ONLY;
	bind();
	if(mMapped)
		return mMapped;
	mMapped = glMapBuffer(mBufferType, access);
	if(mMapped == NULL)
	{
		Utils::CheckGLError("Buffer::map failed!");
	}
	return mMapped;
}

bool Buffer :: unmapBuffer()
{
	bind();
	mMapped = NULL;
	return glUnmapBuffer(mBufferType) != 0;
}

void Buffer :: updateBuffer(int offset, int size, void * data)
{
	bind();
	glBufferSubData(mBufferType, offset, size, data);
}

void Buffer :: Unbind (uint bufferType)
{
	std::map<uint, Buffer *>::iterator it = sBoundBuffers.find(bufferType);

	if(it != sBoundBuffers.end())
	{
		sBoundBuffers.erase(it);
	}

	glBindBuffer(bufferType, 0);
}

} //namespace GL {

} //namespace Squirrel {
