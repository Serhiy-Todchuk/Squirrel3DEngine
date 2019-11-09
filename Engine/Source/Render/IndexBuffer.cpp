#include "IndexBuffer.h"

namespace Squirrel {

namespace RenderData { 

IndexBuffer::IndexBuffer(uint indNum, IndexSize indexSize)
{
	allocate(indNum, indexSize);
	mPolyType = ptTriangles;
	mOrientation = poCounterClockWise;
}

IndexBuffer::~IndexBuffer()
{
	if(mIndices!=0)
	{
		delete[]mIndices;
	}
}

int IndexBuffer::checkIndices(uint32 maxIndexValue)
{
	map(true, false);

	for(uint i = 0; i < getIndicesNum(); ++i)
	{
		uint32 index = getIndex(i);
		if(index > maxIndexValue)
		{
			unmap();
			return static_cast<int>(i);
		}
	}

	unmap();
	return -1;
}

byte* IndexBuffer::getIndexAddr(uint index)
{
	return mIndices + index * mIndexSize;
}

uint32 IndexBuffer::getIndex(uint index)
{
	if(mIndexSize == Index32)
	{
		return *((uint32 *)getIndexAddr(index));
	}
	else
	{
		uint16 i = *((uint16 *)getIndexAddr(index));
		return (uint32)i;
	}
}

void IndexBuffer::setIndex(uint index, uint32 value)
{
	if(mIndexSize == Index32)
	{
		uint32 &i = (uint32&)*(getIndexAddr(index));
		i = value;
	}
	else
	{
		uint16 &i = (uint16&)*(getIndexAddr(index));
		i = (uint16)value;
	}
}

void IndexBuffer::allocate(uint indNum, IndexSize indexSize)
{
	mIndexSize = indexSize;
	mIndNum = indNum;
	mIndices = new byte[indexSize * indNum];
}


void IndexBuffer::quads2triangles()
{
	/*
	if(GetPolyType()!=IndexBuffer::ptQuads)	return;

	VertexBuffer * pVertexBuffer = GetVertexBuffer();

	unsigned int ii=0,jj=0;
	IndexBuffer *	pIndexBuffer = new IndexBuffer((int)(GetIndexBuffer()->GetIndicesNum()*1.5f));
	for(ii=0;ii<pIndexBuffer->GetIndicesNum();ii+=4)
	{
		(*pIndexBuffer)[jj+0]=(*GetIndexBuffer())[ii+0];
		(*pIndexBuffer)[jj+1]=(*GetIndexBuffer())[ii+1];
		(*pIndexBuffer)[jj+2]=(*GetIndexBuffer())[ii+3];
		(*pIndexBuffer)[jj+3]=(*GetIndexBuffer())[ii+3];
		(*pIndexBuffer)[jj+4]=(*GetIndexBuffer())[ii+1];
		(*pIndexBuffer)[jj+5]=(*GetIndexBuffer())[ii+2];
		jj+=6;
	}
	delete GetIndexBuffer();
	m_pIndexBuffer = pIndexBuffer;
	SetPolyType(mtTriangles);
*/
}


}//namespace RenderData { 

}//namespace Squirrel {

