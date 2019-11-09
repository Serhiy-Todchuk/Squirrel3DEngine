#pragma once

#include "BaseImporter.h"
#include <Render/IRender.h>
#include <Resource/Mesh.h>
#include "Arrays.h"
#include "macros.h"
#include <unordered_map>

namespace Squirrel {
namespace DAEImport {
	class Tuple
	{
	public:
		int x;
		int y;
		int z;
		int w;
		int v;

		Tuple() {};
		Tuple(int x_, int y_, int z_) : x(x_), y(y_), z(z_), w(0), v(0) {};
		Tuple(int x_, int y_, int z_, int w_, int v_) : x(x_), y(y_), z(z_), w(w_), v(v_) {};
		
		bool operator<( const Tuple& rhs ) const
		{
			if ( x < rhs.x )	return true;
			if ( x > rhs.x )	return false;
			if ( y < rhs.y )	return true;
			if ( y > rhs.y )	return false;
			if ( z < rhs.z )	return true;
			if ( z > rhs.z )	return false;
			if ( w < rhs.w )	return true;
			if ( w > rhs.w )	return false;
			if ( v < rhs.v )	return true;
			if ( v > rhs.v )	return false;
			return false;
		}
		bool operator==( const Tuple& rhs ) const
		{
			if ( x != rhs.x )	return false;
			if ( y != rhs.y )	return false;
			if ( z != rhs.z )	return false;
			if ( w != rhs.w )	return false;
			if ( v != rhs.v )	return false;
			return true;
		}
	};

}}

namespace std {

template<>
class hash<Squirrel::DAEImport::Tuple> {
public:
	size_t operator()(const Squirrel::DAEImport::Tuple &s) const
	{
		size_t h = std::hash<int>()(s.x);
		h = h ^ (std::hash<int>()(s.y) << 1);
		h = h ^ (std::hash<int>()(s.z) << 1);
		h = h ^ (std::hash<int>()(s.w) << 1);
		h = h ^ (std::hash<int>()(s.v) << 1);
		return h;
	}
};

}

namespace Squirrel {
namespace DAEImport {

using RenderData::VertexBuffer;
using RenderData::IndexBuffer;

class SQDAEIMPORTER_API MeshImporter: public BaseImporter
{
private:

	struct DAEPrimitive
	{
		UniqueId id;
		IntArray * indices;
		IntArray * vcounts;
		IndexBuffer::PolyType polyType;
		int maxOffset;
		int polyCount;
		int positionsOffset;
		int normalsOffset;
		int texcoordsOffset;
		int tangentsOffset;
		int binormalsOffset;
	};


	typedef std::unordered_map<Tuple, int> TupleIndexMap;

public:
	typedef std::vector<int> VERTEX_POSITION_MAP; 

public:
	MeshImporter(Render::IRender * render);
	virtual ~MeshImporter(void);

	virtual void importObject(pugi::xml_node node);

	Resource::Mesh * getMesh(const UniqueId& meshId, const UniqueId& primId);
	VERTEX_POSITION_MAP * getVertexMapping(const UniqueId& meshId);

	std::vector<VertexBuffer *> * getSharedVBs();

private:
	void addTupleIndex( const Tuple& tuple, RenderData::VertexBuffer *pVB, int &iCurrVert, RenderData::IndexBuffer * pIB, int &iCurrInd );
	void addTupleIndex( const DAEPrimitive& primitive, int * tupleArr, VertexBuffer *pVB, int &iCurrVert, IndexBuffer * pIB, int &iCurrInd );

private:

	void addMesh(const UniqueId& meshId, const UniqueId& primId, Resource::Mesh * mesh);

	bool importMesh(pugi::xml_node node);

	Render::IRender * mRender;//for creating VBs and IBs

	TupleIndexMap mTupleMap;
	int mNextTupleIndex;

	UniqueId mDocumentMeshId;

	//parameters of importing
	bool m_bSwapYZ;
	//bool m_bPackIntoOneSharedVB; <- removed this posibility as need to avoid 
	//									packing of skinned and non-skinned VBs into one VB

	std::vector<VertexBuffer *> mSharedVBs;

	typedef std::map<UniqueId, Resource::Mesh *> PRIMITIVES_MAP;
	typedef std::map<UniqueId, PRIMITIVES_MAP> MESHES_MAP;

	//
	MESHES_MAP									mMeshesMap;
	std::map<UniqueId, VERTEX_POSITION_MAP *>	mMeshID2VertexMappingMap;

	FloatArray * mSrcPositions;
	FloatArray * mSrcNormals;
	FloatArray * mSrcTexcoords;
	FloatArray * mSrcTangents;
	FloatArray * mSrcBinormals;

	Math::mat3 mCurrentRotationMatrix;
	Math::vec3 mCurrentTranslationVector;

	VERTEX_POSITION_MAP * mCurrentVertexMapping;
};

}//namespace DAEImport { 
}//namespace Squirrel {