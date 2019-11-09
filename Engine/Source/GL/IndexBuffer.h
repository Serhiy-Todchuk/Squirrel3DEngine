// IndexBuffer.h: interface for the IndexBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Utils.h"
#include "Buffer.h"
#include "Render/IndexBuffer.h"

namespace Squirrel {

namespace GL {

class SQOPENGL_API IndexBuffer:
	public RenderData::IndexBuffer, public Buffer
{
public:
	IndexBuffer(uint indsNum, IndexSize indexSize);
	virtual ~IndexBuffer();

	virtual bool map(bool read, bool write);
	virtual void unmap();

	virtual void update(int offset, int size);

	bool create();

	static void	Unbind();
};


} //namespace GL {

} //namespace Squirrel {
