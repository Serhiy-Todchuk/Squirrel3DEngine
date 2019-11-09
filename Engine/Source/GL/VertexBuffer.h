// IndexBuffer.h: interface for the IndexBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Utils.h"
#include "Buffer.h"
#include "Render/VertexBuffer.h"

namespace Squirrel {

namespace GL { 

class SQOPENGL_API VertexBuffer:
	public RenderData::VertexBuffer, public Buffer
{
public:
	VertexBuffer  (int vertType, int vertsNum);
	~VertexBuffer ();
	
	bool	create ();

	virtual bool	map(bool read, bool write);
	virtual void	unmap();

	virtual void update(int offset, int size);

	static void	Unbind();
};


} //namespace GL {

} //namespace Squirrel {
