#pragma once

#include "Mesh.h"
#include "Skin.h"
#include "MaterialLibrary.h"
#include "TextureStorage.h"
#include "AnimatableResource.h"
#include <Common/Data.h>
#include <list>
#include <memory>

//fixing stupid bug in microsoft stl
#ifdef _WIN32
#	define MEMORY_PTR std::shared_ptr
#else
#	define MEMORY_PTR std::unique_ptr
#endif

namespace Squirrel {

namespace Resource { 

using namespace Math;

//TODO: to move all  classes to namespace
class SQRESOURCE_API Model:
	public StoredObject, 
	public AnimatableResource
{
public://nested types

	struct SQRESOURCE_API MaterialLink
	{
		MaterialLink() :
			mMesh(NULL), mMeshId(-1), 
			idTexDiffuse(-1), idTexSpecular(-1), 
			idTexHeightBump(-1), idMaterial(-1),
			idTexDetail(-1), mMaterial(NULL),
			mIndexOffset(0), mIndexCount(-1)
		{}

		~MaterialLink()
		{
			if(idTexDiffuse >= 0)
				TextureStorage::Active()->release(idTexDiffuse);
			if(idTexSpecular >= 0)
				TextureStorage::Active()->release(idTexSpecular);
			if(idTexHeightBump >= 0)
				TextureStorage::Active()->release(idTexHeightBump);
			if(idTexDetail >= 0)
				TextureStorage::Active()->release(idTexDetail);

			//TODO: release material
		}

		_ID	mMeshId;
		Mesh* mMesh;

		int mIndexOffset;
		int mIndexCount;

		_ID	idTexDiffuse;
		_ID	idTexSpecular;
		_ID	idTexHeightBump;
		_ID	idTexDetail;
		
		_ID	idMaterial;

		Material * mMaterial;

		void generateBumpHeightMap(const std::string& diffTexName);
	};

	typedef std::vector< MaterialLink >		MAT_LINKS_MAP;

	struct Node
	{
		typedef std::list<MEMORY_PTR<Node> > NODE_LIST;

		//constr/destr
		Node(): 
			mSkin(NULL), mSkinId(-1),  
			mID( -1 ), mAnimated(false)
			{ mTransform.identity(); }
		~Node() { }

		_ID	mSkinId;
		Skin* mSkin;

		MAT_LINKS_MAP mMatLinks;

		//TODO: add link to VB
		
		std::string mName;

		mat4 mTransform;

		_ID mID;//former mAnimTargetID

		bool mAnimated;

		NODE_LIST mChildren;
	};

	typedef std::vector< Mesh* >			MESH_MAP;
	typedef std::vector< Skin* >			SKIN_MAP;
	typedef std::vector< VertexBuffer* >	VB_MAP;

public:

	Model(void);
	Model(Mesh * mesh, _ID matId, std::string textureName);
	virtual ~Model(void);

	bool load(Data * data);
	bool save(Data * data);

	void calcBoundingVolume();
	void updateBoundingVolume();

	Node * addNode(Node * parentLink, bool animTarget = true);

	Node::NODE_LIST*		getNodes()			{ return &mNodesList; }
	MESH_MAP*				getMeshes()			{ return &mMeshes; }
	SKIN_MAP*				getSkins()			{ return &mSkins; }

	const Math::AABB&		getAABB() const		{ return mAABB; }

	VB_MAP *				getSharedVBs()		{ return &mSharedVBs;}

	MaterialLibrary *		getLocalMaterials()	{ return mLocalMaterials; }
	void					setLocalMaterials(MaterialLibrary * matLib)	{ 
		DELETE_PTR(mLocalMaterials); mLocalMaterials = matLib; 
	}

	const mat4& getTransform() const { return mTransform; }
	void setTransform(mat4 tform) { mTransform = tform; }

	void prepareForGPUSkinning(int bonesPerVertex, bool addTangentAndBinormal);
	void generateTangentAndBinormal();

	Node * findNode(_ID nodeId);

	template<class _Pred>
	Node * traverseNodes(_Pred pred)
	{
		std::list<Node *> processingNodes;

		Node::NODE_LIST::iterator itNode;

		for(itNode = mNodesList.begin(); itNode != mNodesList.end(); ++itNode )
		{
			processingNodes.push_back(itNode->get());
		}

		while(processingNodes.size() > 0)
		{
			Node * processingNode = processingNodes.front();
			processingNodes.pop_front();

			if(pred(processingNode))
			{
				return processingNode;
			}

			for(itNode = processingNode->mChildren.begin(); itNode != processingNode->mChildren.end(); ++itNode )
			{
				processingNodes.push_back(itNode->get());
			}
		}

		return NULL;
	}

protected:

	bool saveMesh(Data * data, Mesh *mesh);
	bool saveSkin(Data * data, Skin *skin);
	bool saveNode(Data * data, Node *modelNode);
	bool saveVB(Data * data, VertexBuffer *vb);

	Mesh * loadMesh(Data * data);
	Skin * loadSkin(Data * data);
	bool loadNode(Data * data, Node *modelNode);
	VertexBuffer * loadVB(Data * data);

	void destroy();

	void merge(VertexBuffer * vb, Skin * skin, int bonesPerVertex);

	_ID loadTexture(const std::string& texName);

	int getSharedVBIndex(VertexBuffer * vb);

protected:

	Math::AABB				mAABB;
	mat4					mTransform;

	MaterialLibrary *		mLocalMaterials;

	MESH_MAP				mMeshes;
	SKIN_MAP				mSkins;
	Node::NODE_LIST			mNodesList;

	VB_MAP	mSharedVBs;
};

	
}//namespace Resource { 

}//namespace Squirrel {