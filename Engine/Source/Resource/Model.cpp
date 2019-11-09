#include "Model.h"
#include "TextureStorage.h"
#include "ImageLoader.h"
#include <Render/IRender.h>
#include <FileSystem/Path.h>
#include <Common/Settings.h>
#include <set>
#include <stack>

#define _MESH_NODE_ANIM_FLAGS	AnimatableResource::ANIM_TRANSLATE_XYZ | \
								AnimatableResource::ANIM_ROTATE_XYZ | \
								AnimatableResource::ANIM_SCALE_XYZ | \
								AnimatableResource::ANIM_MATRIX4X4

namespace Squirrel {

namespace Resource { 

using namespace FileSystem;

Model::Model(void)
{
	mLocalMaterials		= NULL;
}

Model::Model(Mesh * mesh, _ID matId, std::string textureName)
{
	mLocalMaterials		= NULL;

	mMeshes.push_back(mesh);
	Node * modelNode = addNode(NULL, false);

	modelNode->mMatLinks.push_back(MaterialLink());
	MaterialLink& matLink = modelNode->mMatLinks.back();

	matLink.mMeshId = 0;
	matLink.mMesh = mesh;

	matLink.idMaterial = matId;

	matLink.idTexDiffuse = TextureStorage::Active()->addRetId( textureName );
}

Model::~Model(void)
{
	destroy();
}

void Model::destroy()
{
	uint32 i = 0;

	for(i = 0; i < mMeshes.size(); ++i)
	{
		Mesh * mesh = mMeshes[i];
		DELETE_PTR( mesh );
	}
	mMeshes.clear();

	for(i = 0; i < mSkins.size(); ++i)
	{
		Skin * skin = mSkins[i];
		DELETE_PTR( skin );
	}
	mSkins.clear();

	mNodesList.clear();

	DELETE_PTR(mLocalMaterials);

	for(i = 0; i < mSharedVBs.size(); ++i)
	{
		VertexBuffer * vb = mSharedVBs[i];
		DELETE_PTR(vb);
	}
}

Model::Node * Model::addNode(Model::Node * parentLink, bool animTarget /*= true*/)
{
	//create mesh link
	Node* modelNode;

	if(parentLink != NULL)
	{
		parentLink->mChildren.push_back(MEMORY_PTR<Node>(new Node()));
		modelNode = parentLink->mChildren.back().get();
	}
	else
	{
		mNodesList.push_back(MEMORY_PTR<Node>(new Node()));
		modelNode = mNodesList.back().get();
	}

	modelNode->mAnimated = animTarget;

	//assign id (create animation target)
	modelNode->mID = mTargetsMap.add( _MESH_NODE_ANIM_FLAGS );

	setChanged();

	return modelNode;
}

void Model::calcBoundingVolume()
{
	for(MESH_MAP::iterator it = mMeshes.begin(); it != mMeshes.end(); ++it )
	{
		Mesh * mesh = (*it);
		if(mesh != NULL)
		{
			mesh->calcBoundingVolume();
		}
	}
	updateBoundingVolume();
}

void Model::updateBoundingVolume()
{
	mAABB.reset();
	for(Node::NODE_LIST::iterator it = mNodesList.begin(); it != mNodesList.end(); ++it )
	{
		for(MAT_LINKS_MAP::iterator itMat = (*it)->mMatLinks.begin(); itMat != (*it)->mMatLinks.end(); ++itMat)
		{
			Mesh * mesh = itMat->mMesh;
			if(mesh != NULL)
			{
				Math::AABB meshAABB = mesh->getAABB();
				meshAABB.transform((*it)->mTransform);
				mAABB.merge( meshAABB );
			}
		}
	}
	setChanged();
}

int Model::getSharedVBIndex(VertexBuffer * vb)
{
	for(size_t i = 0; i < mSharedVBs.size(); ++i)
	{
		if(mSharedVBs[i] == vb)
			return i;
	}
	return -1;
}

Model::Node * Model::findNode(_ID nodeId)
{
	struct NodeIDFinder {
		_ID nodeId;
		bool operator() (Node * node) 
		{		
			if(node->mID == nodeId)
				return true;
			return false;
		}
	} nodeIDFinder;

	nodeIDFinder.nodeId = nodeId;

	return traverseNodes(nodeIDFinder);
}

bool Model::load(Data * data)
{
	int i;

	//destroy previous content if exist
	destroy();

	//load local materials if exist
	MaterialLibrary * localMatLib = new MaterialLibrary();
	ASSERT( localMatLib->load( data ) );
	if(localMatLib->getCount() > 0)
	{
		mLocalMaterials = localMatLib;
	}
	else
	{
		delete localMatLib;
	}

	//load shared VB(s)
	int sharedVBsNum = data->readInt32();
	mSharedVBs.resize(sharedVBsNum, NULL);
	for(int i = 0; i < sharedVBsNum; ++i)
	{
		VertexBuffer * sharedVertexBuffer = loadVB(data);
		ASSERT( sharedVertexBuffer != NULL );
		mSharedVBs[i] = sharedVertexBuffer;
	}

	//load meshes
	int meshesNum = data->readInt32();
	for(i = 0; i < meshesNum; ++i)
	{
		Mesh * mesh = loadMesh( data );
		ASSERT( mesh != NULL );
		mMeshes.push_back( mesh );
	}

	//load skins
	int skinsNum = data->readInt32();
	for(i = 0; i < skinsNum; ++i)
	{
		Skin * skin = loadSkin( data );
		ASSERT( skin != NULL );
		mSkins.push_back( skin );
	}

	mTransform = data->readVar<mat4>();

	//load modelNodes
	long modelNodesNum = data->readInt32();
	for(int i = 0; i < modelNodesNum; ++i)
	{
		mNodesList.push_back( MEMORY_PTR<Node>( new Node() ) );
		ASSERT( loadNode( data, mNodesList.back().get() ) );
	}

	//load anims
	ASSERT( loadAnims( data ) );

	//assign IDs for nodes if missed

	struct IDAssigner {
		AnimatableResource * animRes;
		bool operator() (Node * node) 
		{		
			if(node->mID < 0)
				node->mID = animRes->addTarget( _MESH_NODE_ANIM_FLAGS );
			return false;
		}
	} idAssigner;

	idAssigner.animRes = this;

	traverseNodes(idAssigner);

	return true;
}

bool Model::save(Data * data)
{
	uint32 i;

	//save local materials if exist
	if(mLocalMaterials != NULL)
	{
		ASSERT( mLocalMaterials->save( data ) );
	}
	else
	{
		data->putInt32( 0 );
	}

	//save shared VB(s)
	int sharedVBsNum = mSharedVBs.size();
	data->putInt32( sharedVBsNum );
	for(int i = 0; i < sharedVBsNum; ++i)
	{
		VertexBuffer * sharedVertexBuffer = mSharedVBs[i];
		ASSERT( saveVB(data, sharedVertexBuffer) );
	}

	//save meshes
	data->putInt32( mMeshes.size() );
	for(i = 0; i < mMeshes.size(); ++i)
	{
		ASSERT( saveMesh(data, mMeshes[i]) );
	}

	//save skins
	data->putInt32( mSkins.size() );
	for(i = 0; i < mSkins.size(); ++i)
	{
		ASSERT( saveSkin(data, mSkins[i]) );
	}

	data->putVar( mTransform );

	//save modelNodes
	data->putInt32( mNodesList.size() );
	for(Node::NODE_LIST::iterator it = mNodesList.begin(); it != mNodesList.end(); ++it )
	{
		ASSERT( saveNode( data, (*it).get() ) );
	}

	//save anims
	ASSERT( saveAnims( data ) );

	return true;
}

bool Model::saveSkin(Data * data, Skin *skin)
{
	int i = 0;

	//save weights

	data->putInt32( skin->weights.getCount() );
	data->putData( skin->weights.getData(), skin->weights.getCount() * sizeof(float) );

	//save joints

	data->putInt32( skin->joints.getCount() );
	for(i = 0; i < skin->joints.getCount(); ++i)
	{
		Skin::Joint& joint = skin->joints.getData()[ i ];
		data->putByte( joint.getCount() );
		data->putData( joint.getData(), joint.getCount() * sizeof(tuple2i) );
	}

	//save boneNames

	data->putInt32( skin->boneNames.getCount() );
	for(i = 0; i < skin->boneNames.getCount(); ++i)
	{
		data->putString( skin->boneNames.getData()[i] );
	}
	
	return true;
}

Skin * Model::loadSkin(Data * data)
{
	int i;

	Skin * skin = new Skin();
	ASSERT( skin != NULL );

	//load weights

	long weightsNum = data->readInt32();
	skin->weights.setCount( weightsNum );
	data->readBytes( skin->weights.getData(), skin->weights.getCount() * sizeof(float) );

	//load joints

	long jointsNum = data->readInt32();
	skin->joints.setCount( jointsNum );
	for(i = 0; i < skin->joints.getCount(); ++i)
	{
		Skin::Joint& joint = skin->joints.getData()[ i ];
		byte jointSize = data->readByte();
		joint.setCount( jointSize );
		data->readBytes( joint.getData(), joint.getCount() * sizeof(tuple2i) );
	}

	//load boneNames

	long boneNamesNum = data->readInt32();
	skin->boneNames.setCount( boneNamesNum );
	for(i = 0; i < boneNamesNum; ++i)
	{
		std::string& boneName = skin->boneNames.getData()[i];
		data->readString( boneName, '\0' );
	}

	return skin;
}

bool Model::saveMesh(Data * data, Mesh *mesh)
{
	//save index buffer

	data->putVar( mesh->getAABB() );

	IndexBuffer * ib = mesh->getIndexBuffer();
	if(ib == NULL) return false;
	if(ib == NULL)
	{
		//save buffer size. also indicates if buffer is existed (-1 if not)
		data->putInt32( -1 );
		ASSERT( false );
	}
	else
	{
		//save buffer size. also indicates if buffer is existed (-1 if not)
		data->putInt32( (long)ib->getIndicesNum() );	
		
		//save IB info (header)
		data->putInt16 ( (short)ib->getIndexSize()	);
		data->putInt32 ( (long)ib->getPolyOri()	);
		data->putInt32 ( (long)ib->getPolyType()	);

		//save buffer data
		long buffSize = ib->getIndexSize() * ib->getIndicesNum();
		ASSERT(ib->map(true, false));
		data->putData( ib->getIndexBuff(), buffSize );
		ib->unmap();
	}

	//save vertex buffer

	VertexBuffer * vb = mesh->getVertexBuffer();
	if(vb == NULL) return false;
	if(mesh->isSharedVB())
	{
		//shared vb index (-1 if not shared)
		int index = getSharedVBIndex(vb);
		ASSERT(index >= 0);
		data->putInt16( index );
	}
	else
	{
		//shared vb index (-1 if not shared)
		data->putInt16( -1 );
		
		//save VB
		if(!saveVB(data, vb))
		{
			return false;
		}
	}

	return true;
}

Mesh * Model::loadMesh(Data * data)
{
	Math::AABB aabb = data->readVar<Math::AABB>();

	//load index buffer

	//load buffer size. also indicates if buffer is existed (-1 if not)
	int indexNum = data->readInt32();

	if(indexNum <= 0)
	{
		ASSERT( false );
		return NULL;
	}

	short indexSize = data->readInt16();
	
	Render::IRender * render = Render::IRender::GetActive();
	IndexBuffer * ib = render->createIndexBuffer(indexNum, (IndexBuffer::IndexSize)indexSize);
	ASSERT(ib != NULL);
	//ASSERT(indexSize == ib->getIndexSize());

	ib->setPolyOri(  (IndexBuffer::PolyOri)data->readInt32() );
	ib->setPolyType( (IndexBuffer::PolyType)data->readInt32() );
	ib->setStorageType( IBuffer::stGPUStaticMemory );

	//load buffer data
	long buffSize = ib->getIndexSize() * ib->getIndicesNum();
	data->readBytes( ib->getIndexBuff(), buffSize );

	//create mesh

	Mesh * mesh = new Mesh();
	ASSERT(mesh != NULL);
	mesh->setIndexBuffer( ib );

	//load vertex buffer

	short sharedVBIndex = data->readInt16( );

	if(sharedVBIndex < 0)
	{
		//vb is not shared
		//so load vb

		VertexBuffer * vb = loadVB( data );

		ASSERT( vb != NULL );

		mesh->setVertexBuffer( vb );
	}
	else
	{
		ASSERT( static_cast<short>(mSharedVBs.size()) > sharedVBIndex );

		mesh->setSharedVertexBuffer( mSharedVBs[sharedVBIndex] );	
	}

	mesh->setAABB( aabb );

	return mesh;
}

bool Model::saveNode(Data * data, Model::Node *modelNode)
{
	data->putString( modelNode->mName );

	//save mesh and skin pointers

	data->putInt16( static_cast<int16>(modelNode->mSkinId) );

	data->putInt16( modelNode->mMatLinks.size() );

	for(MAT_LINKS_MAP::iterator itMat = modelNode->mMatLinks.begin(); itMat != modelNode->mMatLinks.end(); ++itMat)
	{
		data->putInt16( static_cast<int16>(itMat->mMeshId) );

		//save textures

		Texture * texture = NULL;

		//diffuse
		texture = TextureStorage::Active()->getByID( itMat->idTexDiffuse );
		if(texture)
			data->putString( texture->getName() );
		else
			data->putString("");

		//specular
		texture = TextureStorage::Active()->getByID( itMat->idTexSpecular );
		if(texture)
			data->putString( texture->getName() );
		else
			data->putString("");

		//height/bump
		texture = TextureStorage::Active()->getByID( itMat->idTexHeightBump );
		if(texture)
			data->putString( texture->getName() );
		else
			data->putString("");

		//detail
		texture = TextureStorage::Active()->getByID( itMat->idTexDetail );
		if(texture)
			data->putString( texture->getName() );
		else
			data->putString("");

		//save material

		if(itMat->mMaterial)
			data->putString( itMat->mMaterial->getName() );
		else
			data->putString("");

		//save index buffer offset
		data->putInt32(itMat->mIndexOffset);
		data->putInt32(itMat->mIndexCount);
	}

	//save other data

	data->putVar( modelNode->mTransform );

	data->putInt32( modelNode->mAnimated );

	data->putInt32( modelNode->mID );

	//save children

	data->putInt32( modelNode->mChildren.size() );

	for(Node::NODE_LIST::iterator it = modelNode->mChildren.begin(); it != modelNode->mChildren.end(); ++it )
	{
		ASSERT( saveNode( data, it->get() ) );
	}

	return true;
}

_ID Model::loadTexture(const std::string& texName)
{
	Texture * texture = TextureStorage::Active()->loadTexture( texName );

	if(texture == NULL)
	{
		return _INVALID_ID;
	}

	_ID textureId = texture->getID();

	if(texture->getName() != texName)
	{
		setChanged();
	}

	return textureId;
}

void Model::MaterialLink::generateBumpHeightMap(const std::string& diffTexName)
{
	TextureStorage * texStorage = TextureStorage::Active();
	Texture * tex = texStorage->loadOrGenerateBumpHeightMap(diffTexName);

	//assign
	if(tex)
	{
		idTexHeightBump = tex->getID();
	}
}

bool Model::loadNode(Data * data, Node *modelNode)
{
	std::string nodeName;
	data->readString( nodeName, '\0' );

	modelNode->mName = nodeName;

	//load mesh and skin pointers

	modelNode->mSkinId = data->readInt16();
	if(modelNode->mSkinId >= 0)
	{
		ASSERT( modelNode->mSkinId < mSkins.size() );
		modelNode->mSkin = mSkins[ modelNode->mSkinId ];
	}

	short matLinksNum = data->readInt16();

	for(short i = 0; i < matLinksNum; ++i)
	{
		modelNode->mMatLinks.push_back(MaterialLink());
		MaterialLink& matLink = modelNode->mMatLinks.back();

		matLink.mMeshId = data->readInt16();
		if( matLink.mMeshId >= 0 )
		{
			ASSERT( matLink.mMeshId < mMeshes.size() );
			matLink.mMesh = mMeshes[ matLink.mMeshId ];

			//TODO find better way to setup storage type for vb
			if(modelNode->mSkin == NULL)
			{
				matLink.mMesh->getVertexBuffer()->setStorageType( VertexBuffer::stGPUStaticMemory );
			}
		}

		//load textures

		_ID textureId = -1;
		std::string objName;

		std::string diffTextName;
		_ID diffTexId= -1;

		//diffuse
		data->readString( objName, '\0' );
		if(objName.length() > 0)
		{
			textureId = loadTexture( objName );
			if(textureId >= 0)
				matLink.idTexDiffuse = textureId;
		}
		diffTextName = objName;
		diffTexId = textureId;

		//specular
		textureId = -1;
		data->readString( objName, '\0' );
		if(objName.length() > 0)
		{
			textureId = loadTexture( objName );
			if(textureId >= 0)
				matLink.idTexSpecular = textureId;
		}

		//height/bump
		textureId = -1;
		data->readString( objName, '\0' );
		if(objName.length() > 0)
		{
			textureId = loadTexture( objName );
			if(textureId >= 0)
				matLink.idTexHeightBump = textureId;
		}

		//TODO: move this functionality to editor code
		bool forceNHMGen = Settings::Default()->getInt("Resources", "ForceNHMGen", 1) != 0;
		if(textureId < 0 && forceNHMGen)
		{
			matLink.generateBumpHeightMap(diffTextName);
			if(matLink.idTexHeightBump >= 0)
			{
				setChanged();
			}
		}

		//detail
		textureId = -1;
		data->readString( objName, '\0' );
		if(objName.length() > 0)
		{
			textureId = TextureStorage::Active()->addRetId( objName );
			if(textureId >= 0)
				matLink.idTexDetail = textureId;
		}

		//load material

		MaterialLibrary * matLib = getLocalMaterials();
		if(matLib == NULL)
			matLib = MaterialLibrary::Active();

		data->readString( objName, '\0' );
		matLink.mMaterial = matLib->getByName( objName );
		if(matLink.mMaterial)
			matLink.idMaterial = matLink.mMaterial->getID();

		//load index buffer offset
		if(data->getVersion() >= 101)
		{
			matLink.mIndexOffset = data->readInt32();
			matLink.mIndexCount = data->readInt32();
		}
	}
	//load other data

	modelNode->mTransform = data->readVar<mat4>();

	modelNode->mAnimated = data->readInt32() != 0;

	modelNode->mID = data->readInt32();

	if( modelNode->mID >= 0 )
	{
		mTargetsMap.set( modelNode->mID, _MESH_NODE_ANIM_FLAGS );
	}

	//load children

	long childrenNum = data->readInt32();

	for(int i = 0; i < childrenNum; ++i)
	{
		modelNode->mChildren.push_back( MEMORY_PTR<Node>(new Node()) );
		ASSERT( loadNode( data, modelNode->mChildren.back().get() ) );
	}

	return true;
}

bool Model::saveVB(Data * data, VertexBuffer *vb)
{
	//save vertex buffer

	if(vb == NULL) return false;

	//save VB info (header)
	data->putInt32( vb->getVertsNum() );
	data->putInt32( vb->getVertType() );
		
	//save buffer data
	long buffSize = vb->getVertexSize() * vb->getVertsNum();
	ASSERT(vb->map(true, false));
	data->putData( vb->getVerts(), buffSize );
	vb->unmap();
	
	return true;
}

VertexBuffer * Model::loadVB(Data * data)
{
	//load vertex buffer

	//load VB info (header)
	long vertsNum = data->readInt32();
	long vertType = data->readInt32();

	Render::IRender * render = Render::IRender::GetActive();
	VertexBuffer * vb = render->createVertexBuffer(vertType, vertsNum);
	if(vb == NULL) return NULL;
		
	//save buffer data
	long buffSize = vb->getVertexSize() * vb->getVertsNum();	
	data->readBytes( vb->getVerts(), buffSize );

	return vb;
}

void Model::prepareForGPUSkinning(int bonesPerVertex, bool addTangentAndBinormal)
{
	ASSERT(bonesPerVertex == 4);//now support only 4

	std::set<VertexBuffer *> skinnedVBs;
	std::stack<Node *> processingNodes;

	Node::NODE_LIST::iterator it;
	for(it = mNodesList.begin(); it != mNodesList.end(); ++it )
		processingNodes.push(it->get());

	while(processingNodes.size() > 0)
	{
		Node * node = processingNodes.top();
		processingNodes.pop();

		for(it = node->mChildren.begin(); it != node->mChildren.end(); ++it )
			processingNodes.push(it->get());

		if(node->mSkin == NULL) 
			continue;
		if(node->mMatLinks.size() == 0) 
			continue;
		if(node->mMatLinks[0].mMesh == NULL) 
			continue;

		Mesh * mesh = node->mMatLinks[0].mMesh;
		
		VertexBuffer * oldVB = mesh->getVertexBuffer();

		if(skinnedVBs.find( oldVB ) != skinnedVBs.end()) continue;

		if(oldVB->hasComponent(VertexBuffer::vc4FloatBoneIndices)) continue;

		int newVertexType = oldVB->getVertType();
		newVertexType |= VCI2VT(VertexBuffer::vc4FloatBoneIndices);//check hardware support
		newVertexType |= VCI2VT(VertexBuffer::vc4BoneWeights);//depends on hardware
		if(addTangentAndBinormal)
		{
			newVertexType |= VCI2VT(VertexBuffer::vcTangentBinormal);
		}

		Render::IRender * render = Render::IRender::GetActive();
		VertexBuffer * newVB = render->createVertexBuffer( newVertexType, oldVB->getVertsNum() );

		VertexBuffer::Copy(oldVB, newVB);
		
		merge(newVB, node->mSkin, bonesPerVertex);

		//move content of new buffer to source
		VertexBuffer::MoveContent(newVB, oldVB);

		if(addTangentAndBinormal)
		{
			mesh->calcTangentBasis(mesh->getIndexBuffer(), oldVB);
		}

		oldVB->setStorageType(VertexBuffer::stGPUStaticMemory);

		skinnedVBs.insert(oldVB);

		DELETE_PTR(newVB);

		setChanged();
	}
}

void Model::generateTangentAndBinormal()
{
	std::stack<Node *> processingNodes;

	Node::NODE_LIST::iterator it;
	for(it = mNodesList.begin(); it != mNodesList.end(); ++it )
		processingNodes.push(it->get());

	while(processingNodes.size() > 0)
	{
		Node * node = processingNodes.top();
		processingNodes.pop();

		for(it = node->mChildren.begin(); it != node->mChildren.end(); ++it )
			processingNodes.push(it->get());

		if(node->mMatLinks.size() == 0)
			continue;
		if(node->mMatLinks[0].mMesh == NULL) 
			continue;

		Mesh * mesh = node->mMatLinks[0].mMesh;

		VertexBuffer * oldVB = mesh->getVertexBuffer();

		if(oldVB->hasComponent(VertexBuffer::vcTangentBinormal))
			continue;

		int newVertexType = oldVB->getVertType();
		newVertexType |= VCI2VT(VertexBuffer::vcTangentBinormal);

		Render::IRender * render = Render::IRender::GetActive();
		VertexBuffer * newVB = render->createVertexBuffer( newVertexType, oldVB->getVertsNum() );

		VertexBuffer::Copy(oldVB, newVB);
		
		mesh->calcTangentBasis(mesh->getIndexBuffer(), newVB);

		//move content of new buffer to source
		VertexBuffer::MoveContent(newVB, oldVB);

		oldVB->setStorageType(VertexBuffer::stGPUStaticMemory);

		DELETE_PTR(newVB);

		setChanged();
	}
}

void Model::merge(VertexBuffer * vb, Skin * skin, int bonesPerVertex)
{
	ASSERT(skin->joints.getCount() <= (int)vb->getVertsNum());

	int j = 0, i = 0;
	for(i = 0; i < skin->joints.getCount(); ++i)
	{
		Skin::Joint& joint = skin->joints.getData()[ i ];

		float weightSum = 0;
		int bonesNum = Math::minValue(bonesPerVertex, joint.getCount());
		for(j = 0; j < bonesNum; ++j)
		{
			const tuple2i& boneInfluence = joint.getData()[ j ];
			float weight = skin->weights.getData()[ boneInfluence.x ];
			weightSum += weight;
		}

		vec4 weights(0,0,0,0);
		vec4 boneInds(0,0,0,0);
		for(j = 0; j < bonesNum; ++j)
		{
			const tuple2i& boneInfluence = joint.getData()[ j ];
			weights[j] = skin->weights.getData()[ boneInfluence.x ] / weightSum;
			boneInds[j] = static_cast<float>(boneInfluence.y);
		}
		vb->setComponent<VertexBuffer::vc4FloatBoneIndices>(i, boneInds);
		vb->setComponent<VertexBuffer::vc4BoneWeights>(i, weights);
	}
}

}//namespace Resource { 

}//namespace Squirrel {