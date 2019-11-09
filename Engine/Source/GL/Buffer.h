// IndexBuffer.h: interface for the IndexBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <Render/IBuffer.h>
#include "Utils.h"
#include <common/types.h>
#include "macros.h"

namespace Squirrel {

namespace GL {

using namespace Render;

class SQOPENGL_API Buffer:
	public IContextObject
{
	uint	mBufferType;				// type of buffer object
	uint	mBufferId;					// id of buffer object
	bool	mCreated;
	void *	mMapped;
	
public:
	Buffer(uint bufferType);
	~Buffer ();

	//implement IContextObject

	virtual void generate();
	virtual void destroy();
	
	//implement IBuffer

	bool isCreated() { return mCreated; }

	bool	create(uint bufferSize, void * bufferData, bool dynamic = false);
	void	bind   ();
	void	unbind   ();

	void*	mapBuffer(bool read, bool write);
	bool	unmapBuffer();

	void	updateBuffer(int offset, int size, void * data);

	static void	Unbind(uint bufferType);
};


} //namespace GL {

} //namespace Squirrel {
