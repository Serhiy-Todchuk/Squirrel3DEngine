#include "MeshImporter.h"

namespace Squirrel {

using namespace Math;

namespace DAEImport { 

#define VERTS_TO_INC 100

#define GEOMETRY_TAG	"geometry"
#define MESH_TAG		"mesh"
#define VERTICES_TAG	"vertices"

#define TRIANGLES_TAG	"triangles"
#define TRIFANS_TAG		"trifans"
#define TRISTRIPS_TAG	"tristrips"
#define POLYGONS_TAG	"polygons"
#define POLYLIST_TAG	"polylist"
#define P_INDICES_TAG	"p"
#define VCOUNT_TAG		"vcount"

#define MATERIAL_ATTR		"material"

#define VERTEX_SEMANTIC			"VERTEX"
#define NORMAL_SEMANTIC			"NORMAL"
#define TEXCOORD_SEMANTIC		"TEXCOORD"
#define TEXTANGENT_SEMANTIC		"TEXTANGENT"
#define TEXBINORMAL_SEMANTIC	"TEXBINORMAL"
	

MeshImporter::MeshImporter(Render::IRender * render):
	mRender(render), mSrcPositions(NULL), mSrcNormals(NULL), mSrcTexcoords(NULL),
	m_bSwapYZ(false)
{
}

MeshImporter::~MeshImporter(void)
{
	for(int i = 0; i < mSharedVBs.size(); ++i)
	{
		DELETE_PTR( mSharedVBs[i] );
	}
}

void MeshImporter::importObject(pugi::xml_node node)
{
	if(strcmp(node.name(), GEOMETRY_TAG) != 0) return;

	//
	mDocumentMeshId = node.attribute(ID_ATTR.c_str()).value();

	//meshName = node.attribute(NAME_ATTR.c_str()).value();

	pugi::xml_node meshNode = node.child(MESH_TAG);

	//for now only meshes supported
	if(!meshNode) return;

	importMesh(meshNode);
}


//------------------------------
Resource::Mesh * MeshImporter::getMesh(const UniqueId& meshId, const UniqueId& primId)
{
	MESHES_MAP::const_iterator itMesh = mMeshesMap.find(meshId);
	if(itMesh != mMeshesMap.end())
	{
		const PRIMITIVES_MAP& primMap = itMesh->second;
		PRIMITIVES_MAP::const_iterator itPrim = primMap.find(primId);
		return (itPrim != primMap.end()) ? itPrim->second : NULL;
	}

	return NULL;
}

void MeshImporter::addMesh(const UniqueId& meshId, const UniqueId& primId, Resource::Mesh * mesh)
{
	MESHES_MAP::iterator itMesh = mMeshesMap.find(meshId);

	if(itMesh == mMeshesMap.end())
	{
		mMeshesMap[meshId] = PRIMITIVES_MAP();
		itMesh = mMeshesMap.find(meshId);
	}

	PRIMITIVES_MAP& primMap = itMesh->second;

	primMap[primId] = mesh;
}

MeshImporter::VERTEX_POSITION_MAP * MeshImporter::getVertexMapping(const UniqueId& meshId)
{
	std::map<UniqueId, VERTEX_POSITION_MAP *>::const_iterator it = mMeshID2VertexMappingMap.find(meshId);
	return (it != mMeshID2VertexMappingMap.end()) ? it->second : NULL;
}

std::vector<VertexBuffer *> * MeshImporter::getSharedVBs() 
{ 
	return &mSharedVBs; 
}

bool MeshImporter::importMesh(pugi::xml_node node)
{
	mNextTupleIndex = 0;
	mTupleMap.clear();	

	mSrcPositions = NULL;
	mSrcNormals = NULL;
	mSrcTexcoords = NULL;
	mSrcTangents = NULL;
	mSrcBinormals = NULL;

	std::map<UniqueId, FloatArray *> vertexDataArrays;

	//parse content

	//parse sources
	for(node = node.first_child(); node; node = node.next_sibling())
	{
		//!!! consider that sources are always before vertices, polylist and triangles
		if(SOURCE_TAG != node.name()) break;

		SourceNode * sourceNode = readSourceNode(node);

		if(sourceNode && sourceNode->floatArray)
		{
			vertexDataArrays[sourceNode->id] = sourceNode->floatArray;
			sourceNode->floatArray = NULL;
		}

		DELETE_PTR(sourceNode);
	}

	//parse vertices
	if(String(node.name()) == VERTICES_TAG)
	{
		UniqueId verticesId = node.attribute(ID_ATTR.c_str()).value();

		pugi::xml_node inputNode = node.child(INPUT_TAG.c_str());
		
		UniqueId positionsIdLink = inputNode.attribute(SOURCE_ATTR.c_str()).value();

		UniqueId positionsId = positionsIdLink.substr(1);//skip first character as it is '#'

		std::map<UniqueId, FloatArray *>::iterator it = vertexDataArrays.find(positionsId);
		if(it != vertexDataArrays.end())
		{
			FloatArray * positionsArray = it->second;
			vertexDataArrays.erase(it);						//erase positions
			vertexDataArrays[verticesId] = positionsArray;	//add them as vertices
		}

		node = node.next_sibling();
	}

	std::list<DAEPrimitive> primitives;

	int aproxMaximumVertsNum = 0;

	//parse primitives
	for(; node; node = node.next_sibling())
	{
		bool polylist = false;

		//detect poly type
		IndexBuffer::PolyType polyType = IndexBuffer::ptPoints;//points means NONE
		if(String(node.name()) == TRIANGLES_TAG)
		{
			polyType = IndexBuffer::ptTriangles;
		}
		else if(String(node.name()) == TRIFANS_TAG)
		{
			polyType = IndexBuffer::ptTriFan;
		}
		else if(String(node.name()) == TRISTRIPS_TAG)
		{
			polyType = IndexBuffer::ptTriStrip;
		}
		else if(String(node.name()) == POLYLIST_TAG)
		{
			polyType = IndexBuffer::ptPolygon;
			polylist = true;
		}
		else if(String(node.name()) == POLYGONS_TAG)
		{
			polyType = IndexBuffer::ptPolygon;
			polylist = false;
		}

		//read primitive
		if(polyType != IndexBuffer::ptPoints)
		{
			DAEPrimitive primitive;

			primitive.id = node.attribute(MATERIAL_ATTR).value();
			primitive.polyCount = node.attribute(COUNT_ATTR.c_str()).as_uint();

			primitive.maxOffset = 0;
			primitive.vcounts = NULL;
			primitive.indices = NULL;

			primitive.polyType = polyType;

			for(pugi::xml_node inputNode = node.child(INPUT_TAG.c_str()); inputNode; inputNode = inputNode.next_sibling(INPUT_TAG.c_str()))
			{
				//read attribs

				//read semantic attrib
				String semantic = inputNode.attribute(SEMANTIC_ATTR.c_str()).value();

				//read source attrib
				UniqueId sourceLink = inputNode.attribute(SOURCE_ATTR.c_str()).value();
				UniqueId sourceId = sourceLink.substr(1);//skip first character as it is '#'

				//read offset attrib
				int offset = inputNode.attribute(OFFSET_ATTR.c_str()).as_int();

				//read set attrib
				int set = 0;
				pugi::xml_attribute setAttrib = inputNode.attribute(SET_ATTR.c_str());
				if(setAttrib)
				   set = setAttrib.as_int();

				//find source array
				std::map<UniqueId, FloatArray *>::iterator itSource = vertexDataArrays.find(sourceId);

				//TODO: error
				ASSERT(itSource != vertexDataArrays.end());

				//map source array
				if(semantic == VERTEX_SEMANTIC)
				{
					mSrcPositions = itSource->second;
					primitive.positionsOffset = offset;
				}
				else if(semantic == NORMAL_SEMANTIC)
				{
					mSrcNormals = itSource->second;
					primitive.normalsOffset = offset;
				}
				else if(semantic == TEXCOORD_SEMANTIC)// && set == 0)
				{
					//for now support only first set
					mSrcTexcoords = itSource->second;
					primitive.texcoordsOffset = offset;
				}
				else if(semantic == TEXTANGENT_SEMANTIC)// && set == 0)
				{
					mSrcTangents = itSource->second;
					primitive.tangentsOffset = offset;
				}
				else if(semantic == TEXBINORMAL_SEMANTIC)// && set == 0)
				{
					mSrcBinormals = itSource->second;
					primitive.binormalsOffset = offset;
				}

				if(primitive.maxOffset < offset) primitive.maxOffset = offset;
			}

			pugi::xml_node pNode = node.child(P_INDICES_TAG);

			ASSERT(pNode);//TODO error

			int stride = primitive.maxOffset + 1;

			if(polyType == IndexBuffer::ptPolygon && !polylist)
			{
				//polygons

				primitive.vcounts = new IntArray(primitive.polyCount);

				int i = 0, j = 0;
				std::list<int> indices;

				for(i = 0; i < primitive.polyCount; ++i)
				{
					IntArray polygonIndices(pNode.child_value());

					int polyIndsNum = polygonIndices.getSize() / stride;

					primitive.vcounts->getArray()[i] = polyIndsNum;

					for(j = 0; j < polygonIndices.getSize(); ++j)
					{
						indices.push_back( polygonIndices.getElement(j) );
					}

					pNode = pNode.next_sibling();
				}

				int indicesArraySize = indices.size();
				primitive.indices = new IntArray(indicesArraySize);

				for(i = 0; i < indicesArraySize; ++i)
				{
					primitive.indices->getArray()[i] = indices.front();
					indices.pop_front();
				}

				int entriesCount = indicesArraySize / stride;
				primitive.indices->setAccessorInfo(entriesCount, stride);

				aproxMaximumVertsNum += entriesCount;
			}
			else
			{
				int entriesCount = 0;

				if(polyType == IndexBuffer::ptPolygon)
				{
					//polylist

					pugi::xml_node vcountNode = node.child(VCOUNT_TAG);

					ASSERT(vcountNode);//TODO error

					primitive.vcounts = new IntArray(primitive.polyCount, vcountNode.child_value());

					for(int i = 0; i < primitive.polyCount; ++i)
					{
						entriesCount += primitive.vcounts->getElement(i);
					}
				}
				else
				{
					entriesCount = primitive.polyCount * 3;//TODO!!!
				}

				int indicesArraySize = entriesCount * stride;
				primitive.indices = new IntArray(indicesArraySize, pNode.child_value());
				primitive.indices->setAccessorInfo(entriesCount, stride);

				aproxMaximumVertsNum += entriesCount;
			}

			primitives.push_back(primitive);
		}
	}

	ASSERT(primitives.size());

	ASSERT(mSrcPositions != NULL && mSrcPositions->getSize() > 0);

	// define target vertex type (vertex components)

	int iVertType = VCI2VT(VertexBuffer::vcPosition);

	if(mSrcNormals != NULL && mSrcNormals->getSize() > 0)
	{
		iVertType |= VCI2VT(VertexBuffer::vcNormal);
	}
	if(mSrcTexcoords != NULL && mSrcTexcoords->getSize() > 0)
	{
		iVertType |= VCI2VT(VertexBuffer::vcTexcoord);
	}
	if(mSrcTangents != NULL && mSrcTangents->getSize() > 0)
	{
		iVertType |= VCI2VT(VertexBuffer::vcTangentBinormal);
	}

	//init original vertex position mapping (TODO: make it per VB)
	mCurrentVertexMapping = new VERTEX_POSITION_MAP( aproxMaximumVertsNum );
	mMeshID2VertexMappingMap[ mDocumentMeshId ]	= mCurrentVertexMapping;

	//DAE mesh always has only one VB but can have more IBs(primitives)

	//create VB
	int iCurrVert = 0;
	VertexBuffer * pVB = NULL;

	/*
	if(m_bPackIntoOneSharedVB)
	{
		if(mSharedVBs.size() == 0)
		{
			// create vb with solid vertex, size of vb is approximate minimum, then will be extended if needed
			mSharedVBs.push_back( mRender->createVertexBuffer(iVertType, mSrcPositions->getCount()) );
		}
		else
		{
			iCurrVert = mSharedVBs[0]->getVertsNum();
		}
		pVB = mSharedVBs[0];
	}
	else
	*/

	pVB = mRender->createVertexBuffer(iVertType, aproxMaximumVertsNum);

	IndexBuffer::IndexSize vertexIndexSize = aproxMaximumVertsNum > 65000 ? IndexBuffer::Index32 : IndexBuffer::Index16;

	//iterate through primitives and create mesh for each of them

	for(std::list<DAEPrimitive>::iterator it = primitives.begin(); it != primitives.end(); ++it)
	{
		DAEPrimitive &primitive = (*it);

		//primitive

		//create FS mesh
		Resource::Mesh * pMesh = new Resource::Mesh;

		if(primitives.size() > 1)
		{
			pMesh->setSharedVertexBuffer(pVB);
		}
		else
		{
			pMesh->setVertexBuffer(pVB);
		}

		addMesh(mDocumentMeshId, primitive.id, pMesh);

		IndexBuffer * pIB = NULL;
		int iCurrInd = 0;

		if(primitive.polyType == IndexBuffer::ptPolygon && primitive.vcounts != NULL)
		{
			int polyFirstIndex = 0;
			int vcountInd = 0;

			//calc number of indices
			uint iIndNum = 0;
			for(vcountInd = 0; vcountInd < primitive.vcounts->getCount(); ++vcountInd)
			{
				int vcount = primitive.vcounts->getElement(vcountInd);
				int trianglesCount = vcount - 2;
				iIndNum += trianglesCount * 3;
			}

			//create FS index buffer for mesh
			pIB = pMesh->createIndexBuffer(iIndNum);
			pIB->setPolyType(IndexBuffer::ptTriangles);

			//import indices of polygons and triangulate them
			for(vcountInd = 0; vcountInd < primitive.vcounts->getCount(); ++vcountInd)
			{
				int vcount = primitive.vcounts->getElement(vcountInd);
				int trianglesCount = vcount - 2;

				//fill indices of current IB and extend global (shared) VB if needed
				for(int triangle = 0; triangle < trianglesCount; ++triangle)
				{
					for(int i = 0; i < 3; ++i)
					{
						int index = polyFirstIndex;
						if(i > 0)
						{
							index = polyFirstIndex + i + triangle;
						}

						int * tupleArr = primitive.indices->getTuple(index);

						addTupleIndex(primitive, tupleArr, pVB, iCurrVert, pIB, iCurrInd);
					}
				}

				polyFirstIndex += vcount;
			}
		}
		else
		{
			//create FS index buffer for mesh
			uint iIndNum = primitive.indices->getCount();
			pIB = pMesh->createIndexBuffer(iIndNum, vertexIndexSize);
			pIB->setPolyType(primitive.polyType);

			//fill indices of current IB and extend global (shared) VB if needed
			for(uint index = 0; index < iIndNum; ++index)
			{
				int * tupleArr = primitive.indices->getTuple(index);

				addTupleIndex(primitive, tupleArr, pVB, iCurrVert, pIB, iCurrInd);
			}
		}

		pIB->setPolyOri(IndexBuffer::poNone);

		//optimize size of VB
		if((uint)iCurrVert < pVB->getVertsNum())
		{
			pVB->resize( iCurrVert );
			mCurrentVertexMapping->resize( iCurrVert );
		}
	}

	//clean up

	//remove intermediate vertex data
	std::map<UniqueId, FloatArray *>::iterator itVDA = vertexDataArrays.begin();
	for(; itVDA != vertexDataArrays.end(); ++itVDA)
	{
		DELETE_PTR(itVDA->second);
	}

	//remove intermediate primitives data
	std::list<DAEPrimitive>::iterator itPr = primitives.begin();
	for(; itPr != primitives.end(); ++itPr)
	{
		DELETE_PTR(itPr->indices);
		DELETE_PTR(itPr->vcounts);
	}

	return true;
}

void MeshImporter::addTupleIndex( const DAEPrimitive& primitive, int * tupleArr, VertexBuffer *pVB, int &iCurrVert, IndexBuffer * pIB, int &iCurrInd )
{
	int positionIndex = tupleArr[primitive.positionsOffset];

	int normalIndex = 0;
	if ( mSrcNormals )
		normalIndex = tupleArr[primitive.normalsOffset];

	int uvIndex = 0;
	if ( mSrcTexcoords )
		uvIndex = tupleArr[primitive.texcoordsOffset];

	int tangentIndex = 0;
	if ( mSrcTangents )
		tangentIndex = tupleArr[primitive.tangentsOffset];

	int binormalIndex = 0;
	if ( mSrcBinormals )
		binormalIndex = tupleArr[primitive.binormalsOffset];

	Tuple tuple( positionIndex, normalIndex, uvIndex, tangentIndex, binormalIndex);
	addTupleIndex(tuple, pVB, iCurrVert, pIB, iCurrInd);
}

//------------------------------
void MeshImporter::addTupleIndex( const Tuple& tuple, VertexBuffer *pVB, int &iCurrVert, IndexBuffer * pIB, int &iCurrInd )
{
	TupleIndexMap::iterator it = mTupleMap.find(tuple);
	if ( it == mTupleMap.end() )
	{
		//
		//store vertex index
		//
		//mTupleMap.insert(tuple, mNextTupleIndex);
		mTupleMap[tuple] = mNextTupleIndex;
		pIB->setIndex(iCurrInd++, mNextTupleIndex);
		++mNextTupleIndex;

		if(iCurrVert == pVB->getVertsNum())
		{
			pVB->resize(pVB->getVertsNum() + VERTS_TO_INC);
			mCurrentVertexMapping->resize( pVB->getVertsNum() + VERTS_TO_INC );
		}

		//
		//read vertex position
		//
		Math::vec3 pos;
		float* positionsArray = mSrcPositions->getArray();
		positionsArray += 3 * tuple.x;
		vec3 position(positionsArray[0], positionsArray[1], positionsArray[2]);
		pos = Math::vec3((float)position.x, (float)position.y, (float)position.z);

		mCurrentVertexMapping->operator []( iCurrVert ) = tuple.x;
		pVB->setComponent<VertexBuffer::vcPosition>(iCurrVert, pos);//swap UpZ to UpY later, after applying bind shape transformation

		//
		//read vertex normal
		//
		vec3 normal;
		if ( mSrcNormals )
		{
			float* normalsArray = mSrcNormals->getArray();
			normalsArray += 3 * tuple.y;
			normal = vec3(normalsArray[0], normalsArray[1], normalsArray[2]);
			//normal = mCurrentRotationMatrix * normal;
			normal.normalize();

			pVB->setComponent<VertexBuffer::vcNormal>(iCurrVert, normal);//swap UpZ to UpY later, after applying bind shape transformation
		}

		//
		//read vertex texcoord
		//
		if ( mSrcTexcoords )
		{
			float* uVCoordinateArray = mSrcTexcoords->getArray();
			uVCoordinateArray += mSrcTexcoords->getStride() * tuple.z;
			pVB->setComponent<VertexBuffer::vcTexcoord>(iCurrVert, Math::vec2((float)uVCoordinateArray[0], (float)uVCoordinateArray[1]));
		}

		if(mSrcTangents)
		{
			float* tangentsArray = mSrcTangents->getArray();
			tangentsArray += mSrcTangents->getStride() * tuple.w;
			vec3 tangent(tangentsArray[0], tangentsArray[1], tangentsArray[2]);
			tangent.normalize();

			float dot = 1.0f;

			if(mSrcTangents->getStride() == 4)
				dot = tangentsArray[3];

			if(mSrcBinormals)
			{
				float* binormalsArray = mSrcBinormals->getArray();
				binormalsArray += mSrcBinormals->getStride() * tuple.w;
				vec3 binormal(binormalsArray[0], binormalsArray[1], binormalsArray[2]);
				binormal.normalize();

				vec3 altBinormal = tangent ^ normal;
				dot = altBinormal * binormal;
			}

			pVB->setComponent<VertexBuffer::vcTangentBinormal>(iCurrVert, Math::vec4(tangent, dot));
		}

		++iCurrVert;
	}
	else
	{
		pIB->setIndex(iCurrInd++, it->second);
	}
}

}//namespace DAEImport { 
}//namespace Squirrel {