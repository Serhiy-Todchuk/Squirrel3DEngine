#pragma once

#include <Render/IndexBuffer.h>
#include <Render/VertexBuffer.h>
#include <Math/AABB.h>
#include "macros.h"

namespace Squirrel {

namespace Resource { 

using namespace RenderData;
using namespace Math;

class SQRESOURCE_API Mesh
{
public:

	struct Intesection
	{
		vec3 point;
		vec3 normal;
		int triangleIndex;
	};

public:

	Mesh();
	virtual ~Mesh(void);

	virtual void	render();

	bool findIntersection(vec3 lineStart, vec3 lineEnd, Intesection& out, int startTriangleIndex = 0 );

	void calcTangentBasis(IndexBuffer * ib, VertexBuffer * vb);
	static void calcNormals(IndexBuffer  * ib, VertexBuffer * vb);

	IndexBuffer	* createIndexBuffer(uint iIndNum, IndexBuffer::IndexSize indexSize = IndexBuffer::Index32);
	VertexBuffer	* createVertexBuffer(int vertType, uint vertNum);
	VertexBuffer	* setSharedVertexBuffer(VertexBuffer	*);
	void	setIndexBuffer(IndexBuffer* ib);
	void	setVertexBuffer(VertexBuffer* vb);

	inline IndexBuffer *	getIndexBuffer()  const	{ return m_pIndexBuffer; }
	inline VertexBuffer *	getVertexBuffer() const	{ return m_pVertexBuffer;}

	void calcBoundingVolume();
	void applyTransform(const mat4& transform);

	Math::AABB getAABB() { return mAABB; }
	void setAABB(Math::AABB aabb) { mAABB = aabb; }//for loading from file

	bool isSharedVB() { return m_bSharedVB; }

	static void DrawNormals(VertexBuffer * vb, float length, bool mirror = false);//for debug
	static Mesh * Combine(const Mesh& mesh1, const Mesh& mesh2);

protected:

	Math::AABB				mAABB;
	IndexBuffer *		m_pIndexBuffer;
	VertexBuffer *		m_pVertexBuffer;
	bool					m_bSharedVB;

	std::vector<vec3> mTriangleNormalsCache;
};

class SQRESOURCE_API MeshBuilder
{
public:
	MeshBuilder() {}
	~MeshBuilder() {}

	virtual Mesh * buildMesh() = 0;
};

class SQRESOURCE_API SphereBuilder : 
	public MeshBuilder
{
public:
	SphereBuilder(float fRad_, int iStacks_, int iSlices_, int iVertType_) :
	  fRad(fRad_), iStacks(iStacks_), iSlices(iSlices_), iVertType(iVertType_) {}
	~SphereBuilder() {}

	float fRad;
	int iStacks;
	int iSlices;
	int iVertType;

	virtual Mesh * buildMesh();
};

class SQRESOURCE_API PlaneBuilder :
public MeshBuilder
{
public:
	PlaneBuilder(float sizeX, float sizeZ, int stacks, int slices, int iVertType_) :
		fSizeX(sizeX), fSizeZ(sizeZ), iStacks(stacks), iSlices(slices), iVertType(iVertType_) {}
	~PlaneBuilder() {}

	float fSizeX;
	float fSizeZ;
	int iStacks;
	int iSlices;
	int iVertType;

	virtual Mesh * buildMesh();
};


class SQRESOURCE_API CylinderBuilder : 
	public MeshBuilder
{
public:
	CylinderBuilder(): mHeight(10), mRadTop(4), mRadBottom(7), mStacks(16), mSlices(16), mVertType(VT_PNT) {}
	CylinderBuilder(float height, float fRad1, float fRad2, int iStacks, int iSlices, int iVertType) :
	  mHeight(height), mRadTop(fRad1), mRadBottom(fRad2), mStacks(iStacks), mSlices(iSlices), mVertType(iVertType) {}
	~CylinderBuilder() {}

	float mHeight;
	float mRadTop;
	float mRadBottom;
	int	mStacks;
	int mSlices;
	int mVertType;

	virtual Mesh * buildMesh();
};

class SQRESOURCE_API TorusBuilder : 
	public MeshBuilder
{
public:
	TorusBuilder(): mRadBig(4), mRadSmall(7), mStacks(16), mSlices(16), mVertType(VT_PNT) {}
	TorusBuilder(float fRad1, float fRad2, int iStacks, int iSlices, int iVertType) :
	  mRadBig(fRad1), mRadSmall(fRad2), mStacks(iStacks), mSlices(iSlices), mVertType(iVertType) {}
	~TorusBuilder() {}

	float mRadBig;
	float mRadSmall;
	int	mStacks;
	int mSlices;
	int mVertType;

	virtual Mesh * buildMesh();
};

class SQRESOURCE_API HalfSphereBuilder : 
	public MeshBuilder
{
public:
	HalfSphereBuilder(float fRad_, int iStacks_, int iSlices_, int iVertType_) :
	  fRad(fRad_), iStacks(iStacks_), iSlices(iSlices_), iVertType(iVertType_) {}
	~HalfSphereBuilder() {}

	float fRad;
	int iStacks;
	int iSlices;
	int iVertType;

	virtual Mesh * buildMesh();
};

class SQRESOURCE_API BoxBuilder : 
	public MeshBuilder
{
public:
	BoxBuilder(vec3 min, vec3 max, int vertType) :
	  mMin(min), mMax(max), mVertType(vertType) {}
	~BoxBuilder() {}

	vec3 mMin;
	vec3 mMax;
	int mVertType;

	virtual Mesh * buildMesh();
};

}//namespace Resource { 

}//namespace Squirrel {

