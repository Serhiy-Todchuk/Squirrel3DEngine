#include "Mesh.h"
#include <Math/GeometryTools.h>
#include <Render/IRender.h>

namespace Squirrel {

namespace Resource { 

using namespace Render;

Mesh::Mesh()
{
	m_pIndexBuffer	= NULL;
	m_pVertexBuffer	= NULL;
}

Mesh::~Mesh(void)
{
	DELETE_PTR( m_pIndexBuffer );
	if(!m_bSharedVB)
	{
		DELETE_PTR( m_pIndexBuffer );
	}
}

void Mesh::render()
{
	IRender::GetActive()->setupVertexBuffer( getVertexBuffer() );
	IRender::GetActive()->renderIndexBuffer( getIndexBuffer() );
}

void Mesh::DrawNormals(VertexBuffer * normalsVB, float length, bool mirror)
{
	IRender * render = IRender::GetActive();

	render->setColor(vec4(0.9f, 0.1f, 0.7f, 1));

	size_t i, j;
	size_t vertsNum = normalsVB->getVertsNum() * 2;
	size_t indsNum = vertsNum;

	VertexBuffer * vb = render->createVertexBuffer(VCI2VT(VertexBuffer::vcPosition), vertsNum);
	vb->setStorageType(IBuffer::stCPUMemory);
	IndexBuffer * ib = render->createIndexBuffer(indsNum);
	ib->setStorageType(IBuffer::stCPUMemory);
	ib->setPolyType(IndexBuffer::ptLines);

	for(i = 0, j = 0; i < normalsVB->getVertsNum(); ++i, j+=2)
	{
		vec3 p = normalsVB->getComponent<VertexBuffer::vcPosition>(i);
		vec3 n = normalsVB->getComponent<VertexBuffer::vcNormal>(i) * length;
		vec3 p1 = p + n;
		vec3 p2 = mirror ? p - n : p;

		vb->setComponent<VertexBuffer::vcPosition>(j + 0, p1);
		ib->setIndex(j + 0, j + 0);

		vb->setComponent<VertexBuffer::vcPosition>(j + 1, p2);
		ib->setIndex(j + 1, j + 1);
	}

	render->setupVertexBuffer( vb );
	render->renderIndexBuffer( ib );

	DELETE_PTR(vb);
	DELETE_PTR(ib);


	//glColor3f(1,0,1);
	//for(uint i = 0; i<vb->getVertexSize(); ++i)
	{
		//vec3 p = vb->getComponent<VertexBuffer::vcPosition>(i);
		//vec3 n = vb->getComponent<VertexBuffer::vcNormal>(i) * 2;
		//vec3 p1 = p + n;
		//vec3 p2 = p - n;

		//glBegin(GL_LINES);
		//	glVertex3f(p1.x, p1.y, p1.z);
		//	glVertex3f(p2.x, p2.y, p2.z);
		//glEnd();
	}
	//glColor3f(1,1,1);
}

void Mesh::applyTransform(const mat4& transform)
{
	if(m_pVertexBuffer == NULL) return;
	if(m_pVertexBuffer->getVerts() == NULL) return;

	mat4 normalMarix = transform.inverse().transposed();

	for(int i = 0; i < (int)m_pVertexBuffer->getVertsNum(); ++i)
	{
		vec3 pos = m_pVertexBuffer->getComponent<VertexBuffer::vcPosition>( i );
		pos = transform * pos;
		m_pVertexBuffer->setComponent<VertexBuffer::vcPosition>( i, pos );
		if(m_pVertexBuffer->hasComponent(VertexBuffer::vcNormal))
		{
			vec3 nor = m_pVertexBuffer->getComponent<VertexBuffer::vcNormal>( i );
			nor = normalMarix * nor;
			m_pVertexBuffer->setComponent<VertexBuffer::vcNormal>( i, nor );
		}
	}
}

void Mesh::calcBoundingVolume()
{
	if(m_pVertexBuffer == NULL) return;
	if(m_pVertexBuffer->getVerts() == NULL) return;

	mAABB.reset();
	for(int i = 0; i < (int)m_pVertexBuffer->getVertsNum(); ++i)
	{
		vec3 pos = m_pVertexBuffer->getComponent<VertexBuffer::vcPosition>( i );
		mAABB.addVertex( pos );
	}
}

void Mesh::calcNormals(IndexBuffer  * ib, VertexBuffer * vb)
{
	if(!vb->hasComponent(VertexBuffer::vcNormal)) return;

	if(ib->getIndicesNum()<=0) return;
	if(ib->getPolyType() != IndexBuffer::ptTriangles)	return;

	const uint inc = 3;
	const int vcNorm =  VertexBuffer::vcNormal;

	uint vrt=0;

	uint triNum = ib->getIndicesNum()/inc;
	vec3 * triNorms = new vec3[triNum];

	for(vrt=0; vrt<vb->getVertsNum(); ++vrt )
		vb->getComponent<vcNorm>( vrt ).zero();

	for(uint tri=0; tri<triNum; ++tri )
	{
		vec3 v0 = vb->getComponent<VertexBuffer::vcPosition>( ib->getIndex(tri*inc+0) );
		vec3 v1 = vb->getComponent<VertexBuffer::vcPosition>( ib->getIndex(tri*inc+1) );
		vec3 v2 = vb->getComponent<VertexBuffer::vcPosition>( ib->getIndex(tri*inc+2) );
		triNorms[tri] = ((v1 - v0) ^ (v2 - v0)).normalized();
	}

	int index = 0;
	for(uint ind=0; ind<ib->getIndicesNum(); ++ind )
	{
		index = ib->getIndex(ind);
		vb->setComponent<vcNorm>( index, vb->getComponent<vcNorm>( index ) + triNorms[ (ind - ind%inc)/inc ] );
	}

	for(vrt=0; vrt<vb->getVertsNum(); ++vrt )
	{
		vb->getComponent<vcNorm>( vrt ).normalize();
	}

	delete[]triNorms;
}

void Mesh::calcTangentBasis(IndexBuffer * ib, VertexBuffer * vb)
{
	ASSERT(ib);
	ASSERT(vb);

	ASSERT( ib->getPolyType() == IndexBuffer::ptTriangles );

	ASSERT(vb->hasComponent(VertexBuffer::vcPosition));//must have positions
	ASSERT(vb->hasComponent(VertexBuffer::vcNormal));//must have normals
	ASSERT(vb->hasComponent(VertexBuffer::vcTangentBinormal));//must have tangents
	ASSERT(vb->hasComponent(VertexBuffer::vcTexcoord));//must have texcoords

	int i, j;
	std::vector<vec3> tangents, binormals;	// касательные вектора к треугольнику

	const int facesNum = ib->getIndicesNum() / 3;

	// проходимся по всем треугольникам, и для каждого считаем базис
	for (i = 0; i < facesNum; ++i)
	{
		uint ind0 = ib->getIndex( i * 3 + 0 );
		uint ind1 = ib->getIndex( i * 3 + 1 );
		uint ind2 = ib->getIndex( i * 3 + 2 );

		vec3 v1 = vb->getComponent<VertexBuffer::vcPosition>(ind0);
		vec3 v2 = vb->getComponent<VertexBuffer::vcPosition>(ind1);
		vec3 v3 = vb->getComponent<VertexBuffer::vcPosition>(ind2);
		vec2 t1 = vb->getComponent<VertexBuffer::vcTexcoord>(ind0);
		vec2 t2 = vb->getComponent<VertexBuffer::vcTexcoord>(ind1);
		vec2 t3 = vb->getComponent<VertexBuffer::vcTexcoord>(ind2);

		vec3  t, b;
		calcTriangleTangentBasis( v1, v2, v3, t1, t2, t3, t, b );

		tangents.push_back( t );
		binormals.push_back( b );
	}

	for ( i = 0; i < vb->getVertsNum(); ++i )
	{
		vec3 tangentRes( 0, 0, 0 );
		vec3 binormalRes( 0, 0, 0 );
		int count = 0;
		
		for ( j = 0; j < facesNum; ++j )
		{
			uint ind0 = ib->getIndex( j * 3 + 0 );
			uint ind1 = ib->getIndex( j * 3 + 1 );
			uint ind2 = ib->getIndex( j * 3 + 2 );
			if (ind0 == i || ind1 == i || ind2 == i )
			{
				tangentRes += tangents[ j ];
				binormalRes += binormals[ j ];
				++count;
			}
		}

		tangentRes.safeNormalize();
		binormalRes.safeNormalize();

		vec3 normal = vb->getComponent<VertexBuffer::vcNormal>(i);

		tangentRes	= orthogonalize( normal, tangentRes );
		binormalRes	= orthogonalize( normal, binormalRes );

		float binormalMultiplier = binormalRes * (normal ^ tangentRes);
		vb->setComponent< VertexBuffer::vcTangentBinormal>(i, vec4(tangentRes, binormalMultiplier));
	}
}

IndexBuffer* Mesh::createIndexBuffer( uint iIndNum, IndexBuffer::IndexSize indexSize )
{
	m_pIndexBuffer	= IRender::GetActive()->createIndexBuffer(iIndNum, indexSize);
	ASSERT( m_pIndexBuffer!=NULL );
	return m_pIndexBuffer;
}

VertexBuffer* Mesh::createVertexBuffer(int vertType, uint vertNum)
{
	m_pVertexBuffer	= IRender::GetActive()->createVertexBuffer(vertType, vertNum);
	ASSERT( m_pVertexBuffer!=NULL );
	m_bSharedVB = false;
	return m_pVertexBuffer;
}

VertexBuffer	* Mesh::setSharedVertexBuffer(VertexBuffer	* pVB)
{
	m_pVertexBuffer	= pVB;
	ASSERT( m_pVertexBuffer!=NULL );
	m_bSharedVB = true;
	return m_pVertexBuffer;
}

void Mesh::setIndexBuffer(IndexBuffer* ib)	
{
	DELETE_PTR( m_pIndexBuffer );
	m_pIndexBuffer = ib; 
}

void Mesh::setVertexBuffer(VertexBuffer* vb)	
{ 
	if(!m_bSharedVB)
	{
		DELETE_PTR( m_pVertexBuffer );
	}
	m_bSharedVB = false;
	m_pVertexBuffer = vb;
}

bool Mesh::findIntersection(vec3 lineStart, vec3 lineEnd, Intesection& out, int startTriangleIndex )
{
	int trianglesNum = m_pIndexBuffer->getIndicesNum() * 3;

	//perform some checks
	ASSERT(m_pIndexBuffer->getPolyType() == IndexBuffer::ptTriangles);
	ASSERT(startTriangleIndex < trianglesNum);

	bool recalculateNormals = false;
	if(mTriangleNormalsCache.size() != trianglesNum)
	{
		recalculateNormals = true;
		mTriangleNormalsCache.resize(trianglesNum);
	}

	int index, i;
	vec3 triVerts[3];
	vec3 normal, position;

	for(i = startTriangleIndex; i < trianglesNum; ++i)
	{
		index = i * 3;

		triVerts[0] = m_pVertexBuffer->getComponent<VertexBuffer::vcPosition>( m_pIndexBuffer->getIndex( index + 0 ) );
		triVerts[1] = m_pVertexBuffer->getComponent<VertexBuffer::vcPosition>( m_pIndexBuffer->getIndex( index + 1 ) );
		triVerts[2] = m_pVertexBuffer->getComponent<VertexBuffer::vcPosition>( m_pIndexBuffer->getIndex( index + 2 ) );

		if(recalculateNormals)
		{
			normal = getNormalToTriangle(triVerts);
			mTriangleNormalsCache[i] = normal;
		}
		else
		{
			normal = mTriangleNormalsCache[i];
		}

		bool result = getLnTriIntersect(lineStart, lineEnd, triVerts[0], triVerts[1], triVerts[2], normal, position);

		if(result)
		{
			out.normal = normal;
			out.point = position;
			out.triangleIndex = i;
			return true;
		}
	}

	return false;
}

Mesh * Mesh::Combine(const Mesh& mesh1, const Mesh& mesh2)
{
	VertexBuffer * vb1 = mesh1.getVertexBuffer();
	VertexBuffer * vb2 = mesh2.getVertexBuffer();
	IndexBuffer * ib1 = mesh1.getIndexBuffer();
	IndexBuffer * ib2 = mesh2.getIndexBuffer();

	if(vb1->getVertType() != vb2->getVertType())
		return NULL;

	size_t vb1vertsNum = vb1->getVertsNum();
	size_t vb2vertsNum = vb1->getVertsNum();

	Mesh * mesh = new Mesh();
	VertexBuffer * vb = mesh->createVertexBuffer(vb1->getVertType(), vb1vertsNum + vb2vertsNum);
	IndexBuffer * ib = mesh->createIndexBuffer(ib1->getIndicesNum() + ib2->getIndicesNum());
	ib->setPolyOri(ib1->getPolyOri());
	ib->setPolyType(ib1->getPolyType());

	size_t vb1Size = vb1vertsNum * vb1->getVertexSize();
	size_t vb2Size = vb2vertsNum * vb2->getVertexSize();

	memcpy(vb->getVertexAddr(0), vb1->getVerts(), vb1Size);
	memcpy(vb->getVertexAddr(vb1vertsNum), vb2->getVerts(), vb2Size);

	size_t i, j;

	for(i = 0; i < ib1->getIndicesNum(); ++i)
	{
		ib->setIndex(i, ib1->getIndex(i) );
	}

	for(i = ib1->getIndicesNum(), j = 0; j < ib2->getIndicesNum(); ++j, ++i)
	{
		int oldIndex = ib2->getIndex(j);
		int nexIndex = oldIndex + vb1vertsNum;
		ib->setIndex(i, nexIndex );
	}

	return mesh;
}

Mesh * CylinderBuilder::buildMesh()
{
	//if( (mRadTop <= 0) || (mRadBottom <= 0) || (iStacks<4)||(iSlices<4) ) return 0;

	Mesh * mesh = new Mesh();
	ASSERT(mesh);

	int actV=0;
	int actI=0;
	float height_step = mHeight / (mStacks-1);
	float angle_step = ( 2 * PI ) / ( mSlices - 1 );

	float deltaRadius	= mRadBottom - mRadTop;
	float length		= Math::fsqrt( deltaRadius * deltaRadius + mHeight * mHeight );
	float yNormal		= deltaRadius / length;
	float xzNormalRatio	= mHeight / length;

	///////////////////        CREATE VERTICES      /////////////////////////////////////////////

	VertexBuffer * vb = mesh->createVertexBuffer(mVertType, mStacks*mSlices);

	int st,sl;                                        //vertices
	for(st = 0; st < mStacks;  ++st)
	{
		float stacksFactor = float(st)/(mStacks - 1);
		float radius = Math::lerp(mRadBottom, mRadTop, stacksFactor);

		for(sl = 0; sl < mSlices; ++sl)
		{
			float slicesFactor = float(sl)/(mSlices - 1);

			//vec3 pol(  0,  sl * angle_step,  radius  );

			vec3 pos = vec3::Zero();

			float angle = sl * angle_step;
			pos.x = fsin(angle);
			pos.z = fcos(angle);

			vec3 nor = pos;

			pos *= radius;
			pos.y = height_step * st;

			vb->setComponent<VertexBuffer::vcPosition>(actV, pos);

			if(vb->hasComponent(VertexBuffer::vcTexcoord))
			{
				vb->setComponent<VertexBuffer::vcTexcoord>( actV, vec2(  slicesFactor,  stacksFactor ) );
			}

			if(vb->hasComponent(VertexBuffer::vcNormal))
			{
				nor.normalize();
				nor *= xzNormalRatio;
				nor.y = yNormal;
				vb->setComponent<VertexBuffer::vcNormal>(actV, nor);
			}

			++actV;
		}
	}

	///////////////////        CREATE INDICES      //////////////////////////////////////////////

	IndexBuffer *	ib = mesh->createIndexBuffer((mStacks-1)*(mSlices-1)*6);

	for(st=0; st<mStacks-1;  st++)
	{
		for(sl=0; sl<mSlices-1; sl++)
		{
			ib->setIndex(actI++, (st+0)*(mSlices)+sl+0); // 0
			ib->setIndex(actI++, (st+1)*(mSlices)+sl+0); // 1
			ib->setIndex(actI++, (st+0)*(mSlices)+sl+1); // 3
			ib->setIndex(actI++, (st+0)*(mSlices)+sl+1); // 3
			ib->setIndex(actI++, (st+1)*(mSlices)+sl+0); // 1
			ib->setIndex(actI++, (st+1)*(mSlices)+sl+1); // 2
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	ib->setPolyOri(IndexBuffer::poCounterClockWise);
	ib->setPolyType(IndexBuffer::ptTriangles);

	return mesh;
}

Mesh * PlaneBuilder::buildMesh()
{
	//if( (mRadTop <= 0) || (mRadBottom <= 0) || (iStacks<4)||(iSlices<4) ) return 0;

	Mesh * mesh = new Mesh();
	ASSERT(mesh);

	int actV=0;
	int actI=0;
	float xStep = fSizeX / iStacks;
	float zStep = fSizeZ / iSlices;

	///////////////////        CREATE VERTICES      /////////////////////////////////////////////

	VertexBuffer * vb = mesh->createVertexBuffer(iVertType, iStacks*iSlices);

	int st,sl;                                        //vertices
	for(st = 0; st < iStacks;  ++st)
	{
		float stacksFactor = float(st)/(iStacks - 1);

		for(sl = 0; sl < iSlices; ++sl)
		{
			float slicesFactor = float(sl)/(iSlices - 1);

			vec3 pos;
			pos.y = 0;
			pos.x = -fSizeX * 0.5f + st * xStep;
			pos.z = -fSizeZ * 0.5f + sl * zStep;

			vec3 nor = vec3(0, 1, 0);

			vb->setComponent<VertexBuffer::vcPosition>(actV, pos);

			if(vb->hasComponent(VertexBuffer::vcTexcoord))
			{
				vb->setComponent<VertexBuffer::vcTexcoord>( actV, vec2(  slicesFactor,  stacksFactor ) );
			}

			if(vb->hasComponent(VertexBuffer::vcNormal))
			{
				vb->setComponent<VertexBuffer::vcNormal>(actV, nor);
			}

			++actV;
		}
	}

	///////////////////        CREATE INDICES      //////////////////////////////////////////////

	IndexBuffer *	ib = mesh->createIndexBuffer((iStacks-1)*(iSlices-1)*6);

	for(st=0; st<iStacks-1;  st++)
	{
		for(sl=0; sl<iSlices-1; sl++)
		{
			ib->setIndex(actI++, (st+0)*(iSlices)+sl+0); // 0
			ib->setIndex(actI++, (st+1)*(iSlices)+sl+0); // 1
			ib->setIndex(actI++, (st+0)*(iSlices)+sl+1); // 3
			ib->setIndex(actI++, (st+0)*(iSlices)+sl+1); // 3
			ib->setIndex(actI++, (st+1)*(iSlices)+sl+0); // 1
			ib->setIndex(actI++, (st+1)*(iSlices)+sl+1); // 2
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	ib->setPolyOri(IndexBuffer::poCounterClockWise);
	ib->setPolyType(IndexBuffer::ptTriangles);
	
	return mesh;
}

Mesh * TorusBuilder::buildMesh()
{
	//if( (mRadTop <= 0) || (mRadBottom <= 0) || (iStacks<4)||(iSlices<4) ) return 0;

	Mesh * mesh = new Mesh();
	ASSERT(mesh);

	int actV=0;
	int actI=0;
	float angle_big_step = TWO_PI / (mStacks-1);
	float angle_small_step = TWO_PI / ( mSlices - 1 );

	///////////////////        CREATE VERTICES      /////////////////////////////////////////////

	VertexBuffer * vb = mesh->createVertexBuffer(mVertType, mStacks*mSlices);

	int st,sl;                                        //vertices
	for(st = 0; st < mStacks;  ++st)
	{
		float stacksFactor = float(st)/(mStacks - 1);

		//direction to a center of small ring
		float angle = st * angle_big_step;
		vec3 xzVector(fsin(angle), 0, fcos(angle));

		//center of small ring
		vec3 xzCenter = xzVector * mRadBig;

		//perpendicular to xzVector in xz plane
		vec3 rotAxis(xzVector.z, 0, -xzVector.x);

		for(sl = 0; sl < mSlices; ++sl)
		{
			float slicesFactor = float(sl)/(mSlices - 1);

			//normal to surface
			vec3 nor = xzVector;

			float smallAngle = sl * angle_small_step;
			//rotate normal around big ring
			quat smallRot = quat().fromAxisAngle(rotAxis, smallAngle);
			nor = smallRot.apply(nor);

			//calc pos
			vec3 pos = xzCenter + (nor * mRadSmall);

			vb->setComponent<VertexBuffer::vcPosition>(actV, pos);

			if(vb->hasComponent(VertexBuffer::vcTexcoord))
			{
				vb->setComponent<VertexBuffer::vcTexcoord>( actV, vec2(  slicesFactor,  stacksFactor ) );
			}

			if(vb->hasComponent(VertexBuffer::vcNormal))
			{
				vb->setComponent<VertexBuffer::vcNormal>(actV, nor);
			}

			++actV;
		}
	}

	///////////////////        CREATE INDICES      //////////////////////////////////////////////

	IndexBuffer *	ib = mesh->createIndexBuffer((mStacks-1)*(mSlices-1)*6);

	for(st=0; st<mStacks-1;  st++)
	{
		for(sl=0; sl<mSlices-1; sl++)
		{
			ib->setIndex(actI++, (st+0)*(mSlices)+sl+0); // 0
			ib->setIndex(actI++, (st+1)*(mSlices)+sl+0); // 1
			ib->setIndex(actI++, (st+0)*(mSlices)+sl+1); // 3
			ib->setIndex(actI++, (st+0)*(mSlices)+sl+1); // 3
			ib->setIndex(actI++, (st+1)*(mSlices)+sl+0); // 1
			ib->setIndex(actI++, (st+1)*(mSlices)+sl+1); // 2
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	ib->setPolyOri(IndexBuffer::poCounterClockWise);
	ib->setPolyType(IndexBuffer::ptTriangles);

	return mesh;
}

Mesh * SphereBuilder::buildMesh()
{
	if( (fRad==0)||(iStacks<4)||(iSlices<4) ) return 0;

	Mesh * mesh = new Mesh();
	ASSERT(mesh);

	int actV=0;
	int actI=0;
	float angle_step_1=float(PI)/(iStacks-1);
	float angle_step_2=float(2*PI)/(iSlices-1);
	///////////////////        CREATE VERTICES      /////////////////////////////////////////////
	mesh->createVertexBuffer(iVertType, iStacks*iSlices);
	VertexBuffer * pVertexBuffer = mesh->getVertexBuffer();
	int st,sl;                                        //vertices
	for(st=0; st<iStacks;  ++st)
	{
		for(sl=0; sl<iSlices; ++sl)
		{
			vec3 pol(  st*angle_step_1-float(HALF_PI),  sl*angle_step_2,  fRad  );
			vec3 pos = vec3::Polar2Cartesian(pol);

			pVertexBuffer->setComponent<VertexBuffer::vcPosition>(actV, pos);

			if(pVertexBuffer->hasComponent(VertexBuffer::vcTexcoord))
			{
				pVertexBuffer->setComponent<VertexBuffer::vcTexcoord>(actV, 
					vec2(  (float)sl/(float)(iSlices-1),  (float)st/(float)(iStacks-1)));
			}

			if(pVertexBuffer->hasComponent(VertexBuffer::vcNormal))
			{
				pos.normalize();
				pVertexBuffer->setComponent<VertexBuffer::vcNormal>(actV, pos);
			}

			++actV;
		}
	}

	///////////////////        CREATE INDICES      //////////////////////////////////////////////
	mesh->createIndexBuffer((iStacks-1)*(iSlices-1)*6);
	IndexBuffer *	pIndexBuffer = mesh->getIndexBuffer();
	for(st=0; st<iStacks-1;  st++)
	{
		for(sl=0; sl<iSlices-1; sl++)
		{
			pIndexBuffer->setIndex(actI++, (st+0)*(iSlices)+sl+0); // 0
			pIndexBuffer->setIndex(actI++, (st+1)*(iSlices)+sl+0); // 1
			pIndexBuffer->setIndex(actI++, (st+0)*(iSlices)+sl+1); // 3
			pIndexBuffer->setIndex(actI++, (st+0)*(iSlices)+sl+1); // 3
			pIndexBuffer->setIndex(actI++, (st+1)*(iSlices)+sl+0); // 1
			pIndexBuffer->setIndex(actI++, (st+1)*(iSlices)+sl+1); // 2
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	pIndexBuffer->setPolyOri(IndexBuffer::poClockWise);
	pIndexBuffer->setPolyType(IndexBuffer::ptTriangles);

	return mesh;
}

Mesh * HalfSphereBuilder::buildMesh()
{
	if( (fRad==0)||(iStacks<4)||(iSlices<4) ) return 0;
	int stacks_=(iStacks/3)*2;


	Mesh * mesh = new Mesh();
	ASSERT(mesh);

	int actV=0;
	int actI=0;
	float angle_step_1=float(PI)/(iStacks-1);
	float angle_step_2=float(2*PI)/(iSlices-1);
	///////////////////        CREATE VERTICES      /////////////////////////////////////////////
	mesh->createVertexBuffer(iVertType, stacks_*iSlices);
	VertexBuffer * pVertexBuffer = mesh->getVertexBuffer();
	int st,sl;                                        //vertices
	for(st = iStacks - stacks_; st < iStacks;  ++st)
	{
		for(sl=0; sl<iSlices; ++sl)
		{
			vec3 pol(  st*angle_step_1-float(HALF_PI),  sl*angle_step_2,  fRad  );
			vec3 pos = vec3::Polar2Cartesian(pol);

			pVertexBuffer->setComponent<VertexBuffer::vcPosition>(actV, pos);

			if(pVertexBuffer->hasComponent(VertexBuffer::vcTexcoord))
			{
				pVertexBuffer->setComponent<VertexBuffer::vcTexcoord>(actV, 
					vec2(  (float)sl/(float)(iSlices-1),  (float)st/(float)(iStacks-1)));
			}

			if(pVertexBuffer->hasComponent(VertexBuffer::vcNormal))
			{
				pos.normalize();
				pVertexBuffer->setComponent<VertexBuffer::vcNormal>(actV, pos);
			}

			++actV;
		}
	}

	///////////////////        CREATE INDICES      //////////////////////////////////////////////
	mesh->createIndexBuffer((stacks_-1)*(iSlices-1)*6);
	IndexBuffer *	pIndexBuffer = mesh->getIndexBuffer();
	for(st=0; st<stacks_-1;  st++)
	{
		for(sl=0; sl<iSlices-1; sl++)
		{
			pIndexBuffer->setIndex(actI++, (st+0)*(iSlices)+sl+0); // 0
			pIndexBuffer->setIndex(actI++, (st+1)*(iSlices)+sl+0); // 1
			pIndexBuffer->setIndex(actI++, (st+0)*(iSlices)+sl+1); // 3
			pIndexBuffer->setIndex(actI++, (st+0)*(iSlices)+sl+1); // 3
			pIndexBuffer->setIndex(actI++, (st+1)*(iSlices)+sl+0); // 1
			pIndexBuffer->setIndex(actI++, (st+1)*(iSlices)+sl+1); // 2
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	pIndexBuffer->setPolyOri(IndexBuffer::poClockWise);
	pIndexBuffer->setPolyType(IndexBuffer::ptTriangles);

	return mesh;
}

Mesh * BoxBuilder::buildMesh()
{
	Mesh * mesh = new Mesh();
	ASSERT(mesh);
	mesh->createVertexBuffer(mVertType, 8);
	VertexBuffer * pVB = mesh->getVertexBuffer();

	int actV=0;
	//////////////////////////////////////////
	pVB->setComponent<VertexBuffer::vcPosition>(actV, vec3(mMin.x, mMin.y, mMax.z));	actV++;
	pVB->setComponent<VertexBuffer::vcPosition>(actV, vec3(mMax.x, mMin.y, mMax.z));	actV++;
	pVB->setComponent<VertexBuffer::vcPosition>(actV, vec3(mMax.x, mMax.y, mMax.z));	actV++;
	pVB->setComponent<VertexBuffer::vcPosition>(actV, vec3(mMin.x, mMax.y, mMax.z));	actV++;
	pVB->setComponent<VertexBuffer::vcPosition>(actV, vec3(mMin.x, mMin.y, mMin.z));	actV++;
	pVB->setComponent<VertexBuffer::vcPosition>(actV, vec3(mMin.x, mMax.y, mMin.z));	actV++;
	pVB->setComponent<VertexBuffer::vcPosition>(actV, vec3(mMax.x, mMax.y, mMin.z));	actV++;
	pVB->setComponent<VertexBuffer::vcPosition>(actV, vec3(mMax.x, mMin.y, mMin.z));	actV++;

	if(pVB->hasComponent(VertexBuffer::vcNormal))
	{
		for(int i = 0; i < 8; ++i)
		{
			vec3 pos = pVB->getComponent<VertexBuffer::vcPosition>(i);
			pos.normalize();
			pVB->setComponent<VertexBuffer::vcNormal>(i, pos);
		}
	}

	//////////////////////////////////////////
	mesh->createIndexBuffer(24);
	IndexBuffer *	pIB = mesh->getIndexBuffer();

	short * inds = reinterpret_cast<short *>(pIB->getIndexBuff());
	{inds[0]=0;		inds[1]=1;	inds[2]=2;	inds[3]=3; }
	{inds[4]=4;		inds[5]=5;	inds[6]=6;	inds[7]=7; }
	{inds[8]=5;		inds[9]=3;	inds[10]=2;	inds[11]=6;}
	{inds[12]=4;	inds[13]=7;	inds[14]=1;	inds[15]=0;}
	{inds[16]=7;	inds[17]=6;	inds[18]=2;	inds[19]=1;}
	{inds[20]=4;	inds[21]=0;	inds[22]=3;	inds[23]=5;}

	pIB->setPolyOri(IndexBuffer::poNone);
	pIB->setPolyType(IndexBuffer::ptQuads);

	return mesh;
}


}//namespace Resource { 

}//namespace Squirrel {
