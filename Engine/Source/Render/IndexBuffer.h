#pragma once

#include <Common/types.h>
#include <Math/BasicUtils.h>
#include "macros.h"
#include "IBuffer.h"

namespace Squirrel {

namespace RenderData { 

class SQRENDER_API IndexBuffer: public IBuffer
{
	friend class Mesh;

public:

	enum PolyType
	{
		ptPoints = 0x0000,
		ptLines,
		ptLineLoop,
		ptLineStrip,
		ptTriangles,
		ptTriStrip,
		ptTriFan,
		ptQuads,
		ptQuadStrip,
		ptPolygon
	};

	enum PolyOri
	{
		poNone = 0x0000,
		poClockWise = 0x0900,
		poCounterClockWise = 0x0901
	};

	enum IndexSize
	{
		Index16 = 2,
		Index32 = 4
	};

protected:
	IndexBuffer(uint indNum, IndexSize indexSize);
public:
	virtual ~IndexBuffer();

	byte* getIndexAddr(uint index);
	uint32  getIndex(uint index);
	void setIndex(uint index, uint32 value);

	//returns negative value if indices are ok, 
	//otherwise returns index of index which has too big value
	int checkIndices(uint32 maxIndexValue);

	inline uint			getIndicesNum()		{ return mIndNum; }
	inline IndexSize	getIndexSize()		{ return mIndexSize; }
	inline byte*		getIndexBuff()		{ return mIndices; }

	inline PolyOri		getPolyOri()				{return mOrientation;}
	inline PolyType		getPolyType()				{return mPolyType;}

	inline void			setPolyOri(PolyOri mo)		{mOrientation = mo;}
	inline void			setPolyType(PolyType mt)	{mPolyType = mt;}

protected:
	void allocate(uint indNum, IndexSize indexSize);
	void quads2triangles();

protected:

	byte*		mIndices;
	uint		mIndNum;
	IndexSize   mIndexSize;

	PolyOri		mOrientation;
	PolyType	mPolyType;
};


}//namespace RenderData { 

}//namespace Squirrel {

