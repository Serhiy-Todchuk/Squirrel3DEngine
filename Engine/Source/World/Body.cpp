#include "Body.h"
#include <Resource/TextureStorage.h>
#include <Resource/ModelStorage.h>
#include <Resource/AnimationRunner.h>
#include <Reflection/AtomicWrapper.h>
#include <Common/Log.h>
#include "Skeleton.h"
#include <sstream>

namespace Squirrel {

namespace World { 

Render::UniformString sUniformDecalMap			("decalMap");
Render::UniformString sUniformNormalHeightMap	("normalHeightMap");
Render::UniformString sUniformSpecularMap		("specularMap");
Render::UniformString sUniformDetailMap			("detailMap");

SQREFL_REGISTER_CLASS_SEED(World::Body, WorldBody);

Body::Body()
{
	initMembers();

	SQREFL_SET_CLASS(World::Body);

	Reflection::Object::Field * field = wrapAtomicField("ModelRoot",	&mModelRoot);
	field->attributes.push_back("Read-only");

	field = wrapAtomicField("MeshId",		&mMeshId);
	field->attributes.push_back("Read-only");

	field = wrapAtomicField("ModelName",	&mModelName);
	field->attributes.push_back("Read-only");
}

Body::~Body(void)
{
	if(mModel != NULL)
	{
		ModelStorage::Active()->release( mModel->getID() );
	}

	if(mMesh != NULL && mMeshOwner)
	{
		for(size_t i = 0; i < mMesh->mMatLinks.size(); ++i)
			DELETE_PTR( mMesh->mMatLinks[i].mMesh );
		DELETE_PTR( mMesh->mSkin );
		DELETE_PTR( mMesh );
	}

	DELETE_PTR( mSkeleton );
}

void Body::initMembers()
{
	//content members

	mMeshOwner			= false;

	mModel				= NULL;
	mMesh				= NULL;
	mSkeleton			= NULL;

	mModelRoot			= false;
	mMeshId				= _INVALID_ID;
}


bool Body::findAllIntersections(Ray ray, RAYCASTHITS_LIST& outList)
{
	bool result = SceneObjectsContainer::findAllIntersections(ray, outList);

	vec3 normal, point;
	vec3 lineEnd = ray.mOrigin + ray.mDirection * 100000.0f;

	if(getMesh() != NULL)
	{
		AABB bounds = getAABB();

		if(bounds.getIntersection(ray.mOrigin, lineEnd, normal, point))
		{
			outList.push_back(RaycastHit());
			outList.back().obj = this;
			outList.back().position = point;
			outList.back().normal = normal;
			result = true;

			//TODO: check intersection with mesh
		}
	}

	return result;
}

void Body::addChildren(Model::Node::NODE_LIST * sourceMeshNodes, Body * modelRoot, BODIES_LIST& bodiesWithSkeletons)
{
	ASSERT( sourceMeshNodes );
	ASSERT( modelRoot );

	Model::Node::NODE_LIST::iterator itMeshNode = sourceMeshNodes->begin();
	for(;itMeshNode != sourceMeshNodes->end(); ++itMeshNode)
	{
		Model::Node * meshNode = itMeshNode->get();

		//create child scene object
		Body * newChild			= new Body();
		newChild->setName(meshNode->mName);
		newChild->mMaster		= modelRoot;
		newChild->mModelName	= modelRoot->mModelName;
		newChild->mModelRoot	= false;
		newChild->mMesh			= meshNode;
		newChild->mMeshId		= meshNode->mID;
		newChild->setLocalTransform( meshNode->mTransform );
		newChild->getLocalPosition();

		modelRoot->setupSubordinate(newChild, bodiesWithSkeletons);

		addSceneObject(newChild);

		//add children
		newChild->addChildren(&meshNode->mChildren, modelRoot, bodiesWithSkeletons);
	}
}

void Body::setupSubordinate(Body * subordinate, BODIES_LIST& bodiesWithSkeletons)
{
	ASSERT(subordinate);
	ASSERT(subordinate->mMesh);

	//setup anim target for it
	if( subordinate->mMesh->mAnimated )
	{
		Animatable::TargetInfo * animTarget = new Animatable::TargetInfo();
		animTarget->mComponentsNum = 16;
		animTarget->mTargetData = &subordinate->mLocalTransform.x.x;
		animTarget->mChangeFlags = new bool*[ animTarget->mComponentsNum ];
		for(int i = 0; i < 16;++i) animTarget->mChangeFlags[i] = &subordinate->mTransformChanged;
		mTargetsMap.set( subordinate->mMesh->mID, animTarget );
	}

	//setup skeleton
	if(subordinate->mMesh->mSkin != NULL && subordinate->mMesh->mMatLinks.size() > 0 && subordinate->mMesh->mMatLinks[0].mMesh != NULL)
	{
		subordinate->mSkeleton = new Skeleton();

		subordinate->mSkeleton->initVB( subordinate->mMesh->mMatLinks[0].mMesh->getVertexBuffer() );

		bodiesWithSkeletons.push_back(subordinate);
	}
}

void Body::initWithMesh(Resource::Mesh * mesh)
{
	mMesh = new Model::Node();
	mMesh->mMatLinks.push_back(Model::MaterialLink());
	Model::MaterialLink& matLink = mMesh->mMatLinks.back();
	matLink.mMeshId = -1;
	matLink.mMesh = mesh;
	mMeshOwner = true;
}

void Body::initWithModel(Model * sourceModel)
{
	mModel		= sourceModel;

	if(mModel != NULL)
	{
		mModelName	= sourceModel->getName();
		mModelRoot	= true;

		if(getName().empty()) setName(mModel->getName());

		BODIES_LIST bodiesWithSkeletons;

		addChildren( mModel->getNodes(), this, bodiesWithSkeletons );

		setupMasterAnimations(bodiesWithSkeletons);

		setLocalTransform(mModel->getTransform());
	}

	getLocalPosition();

	updateTransform();
}

void Body::onModelNameChanged()
{
	//TODO: release children based on this model
	//TODO: reset members

	if(mModel != NULL)
	{
		ModelStorage::Active()->release( mModel->getID() );
	}

	mModel = ModelStorage::Active()->add( mModelName );

	initWithModel(mModel);
}

void Body::setupMasterAnimations(const BODIES_LIST& bodiesWithSkeletons)
{
	ASSERT(mModel);
	ASSERT(mModelRoot);

	for(BODIES_LIST::const_iterator itSkel = bodiesWithSkeletons.begin(); itSkel != bodiesWithSkeletons.end(); ++itSkel)
	{
		Body * bodyWithSkeleton = (*itSkel);
		bodyWithSkeleton->mSkeleton->fetchBones(bodyWithSkeleton->mMesh->mSkin, this);
	}

	if(	mModel->getAnimations().size() > 0)
	{
		//instantiate animations
		mAnimResource = TYPE_CAST<AnimatableResource *>( mModel );
	
		if(mAnimations.get() == NULL)
		{
			mAnimations.reset( new AnimationRunner );
		}

		AnimatableResource::ANIMS_MAP::const_iterator itAnims;
		for(itAnims = mModel->getAnimations().begin(); itAnims != mModel->getAnimations().end(); ++itAnims)
		{
			Animation * animRes = itAnims->second;

			if(animRes->getSubAnims()->size() == 0)
			{
				AnimationNode * animNode = new AnimationNode(animRes);
				animNode->setTarget( this );
				mAnimations->addAnimNode( itAnims->first, animNode );
			}

			Animation::SUBANIMS_MAP::iterator itSubAnims;
			for(	itSubAnims = animRes->getSubAnims()->begin(); 
					itSubAnims != animRes->getSubAnims()->end();
					++itSubAnims)
			{
				AnimationNode * animNode = new AnimationNode(animRes, &itSubAnims->second);
				animNode->setTarget( this );
				mAnimations->addAnimNode( itSubAnims->first, animNode );
			}
		}

		mAnimations->playAnimNode("main");
	}
}

void Body::deserialize(Reflection::Deserializer * deserializer)
{
	SceneObject::deserialize(deserializer);

	if(mModelName.length() > 0 && mModelRoot)
	{
		mModel = ModelStorage::Active()->add( mModelName );		
	}

	if(mModelRoot && mModel != NULL)
	{
		BODIES_LIST bodiesWithSkeletons;

		SCENE_OBJECTS_LIST descendants(mSceneObjects);
		while(descendants.size() > 0)
		{
			SceneObject * descendant = descendants.front();
			descendants.pop_front();

			if(descendant->isKindOfClass("World::Body"))
			{
				Body * body = static_cast<Body *>(descendant);

				if(body->mModelName == mModelName)
				{
					body->mMesh			= mModel->findNode(body->mMeshId);
					body->mMeshOwner	= false;
					body->mMaster		= this;
					setupSubordinate(body, bodiesWithSkeletons);
				}
			}

			descendants.insert(descendants.end(), descendant->getSceneObjects().begin(), descendant->getSceneObjects().end());
		}

		setupMasterAnimations(bodiesWithSkeletons);
	}
}

void Body::saveSubAnims()
{
	if(mAnimations.get() == NULL) 
		return;

	if(mModel == NULL)
		return;

	if(mModel->getAnimations().size() == 0)
		return;

	//for now support only one anim per model!!!
	//so take first anim
	Animation * dstAnim = mModel->getAnimations().begin()->second;

	dstAnim->getSubAnims()->clear();

	AnimationRunner::ANIMS_MAP::const_iterator itSrc = mAnimations->getAnimNodes().begin();
	for(; itSrc != mAnimations->getAnimNodes().end(); ++itSrc)
	{
		AnimationNode * srcSubAnim = itSrc->second;

		Animation::SubAnim& dstSubAnim = dstAnim->newSubAnim( srcSubAnim->getName() );
		dstSubAnim.mName		= srcSubAnim->getName();
		dstSubAnim.mTimeRange	= srcSubAnim->getTimeRange();
		dstSubAnim.mRepeats		= srcSubAnim->getRepeatsNum();
	}

	mModel->setChanged();
}

void Body::update(float dtime)
{
	if(mSkeleton != NULL && mMesh != NULL)
	{
		//mSkeleton->build( );
	}
}

void Body::renderDebugInfo(Render::IRender * render, Render::Camera * camera)
{
	if(mSkeleton != NULL)
	{
		mSkeleton->render( render );
	}

	//SceneObject::renderDebugInfo(render, camera);
}

void Body::render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info)
{
	if(mMesh == NULL)				
		return;
	else if(mMesh->mMatLinks.size() == 0)
		return;

	float lod = Math::maxValue( 0.0f, info.minLOD );

	for(size_t i = 0; i < mMesh->mMatLinks.size(); ++i)
	{
		Model::MaterialLink& matLink = mMesh->mMatLinks[i];

		ASSERT(matLink.mMesh);
		ASSERT(matLink.mMesh->getIndexBuffer());

		//describe material

		Render::MaterialGroup * matGroup = renderQueue->beginMaterialGroup();

		std::stringstream programParams;

		Texture * decalMap = NULL;
		if(info.level >= rilDecalMaps)
		{
			if(matLink.idTexDiffuse >= 0)
			{
				decalMap = TextureStorage::Active()->getByID(matLink.idTexDiffuse);
				if(decalMap)
				{
					matGroup->mTextures[sUniformDecalMap] = decalMap->getRenderTexture();
				}
			}
			if (!decalMap)
			{
				programParams << "NOTEXTURES;";
			}
		}
		if(info.level >= rilMaterials)
		{
			if(lod < sLODGoodForReflections && decalMap)
			{
				Texture * bumpMap = NULL;
				Texture * specMap = NULL;
				if(matLink.idTexHeightBump >= 0)
				{
					bumpMap = TextureStorage::Active()->getByID(matLink.idTexHeightBump);
					if(bumpMap)
					{
						matGroup->mTextures[sUniformNormalHeightMap] = bumpMap->getRenderTexture();
						programParams << "BUMP;";
					}
				}
				if(matLink.idTexSpecular >= 0)
				{
					specMap = TextureStorage::Active()->getByID(matLink.idTexSpecular);
					if(specMap)
					{
						matGroup->mTextures[sUniformSpecularMap] = specMap->getRenderTexture();
						programParams << "SPECULAR_MAP;";
					}
				}
			}

			if(matLink.mMaterial != NULL)
			{
				matGroup->mMaterial = matLink.mMaterial;
			}
		}

		//get vb and bones data

		VertexBuffer * vb = NULL;
		int bonesCount = 0;
		vec4 * bonesData = NULL;

		if(mSkeleton != NULL && mSkeleton->getVertexBuffer() != NULL)
		{
			//TODO: apply only once per VB
			mSkeleton->applyCPUSkinning( matLink.mMesh->getVertexBuffer(), mMesh->mSkin );
			vb = mSkeleton->getVertexBuffer();
		}
		else
		{
			if(mSkeleton != NULL)
			{
				mSkeleton->buildGPUData();

				//GPU skinning
				programParams		<< "SKINNING;";
				bonesCount			= mSkeleton->getGPUBonesData().getCount();
				bonesData			= mSkeleton->getGPUBonesData().getData();
			}
			vb = matLink.mMesh->getVertexBuffer();
		}

		matGroup->mProgramParams = programParams.str();

		matGroup = renderQueue->endMaterialGroup();

		Render::VBGroup * vbGroup = matGroup->getVBGroup(vb, bonesCount);
		vbGroup->mBonesData = bonesData;

		//put index primitive

		Render::IndexPrimitive * primitive = vbGroup->getIndexPrimitive(matLink.mMesh->getIndexBuffer());

		mat4 transform = mTransform;
		if(mSkeleton != NULL)
		{
			transform = mat4::Identity();
		}

		primitive->addInstance( transform, mAABB );
	}
}

void Body::calcAABB()
{
	if(mMesh != NULL)
	{
		mAABB.reset();

		for(size_t i = 0; i < mMesh->mMatLinks.size(); ++i)
		{
			AABB meshAABB = mMesh->mMatLinks[i].mMesh->getAABB();
			meshAABB.transform( mTransform );
			mAABB.merge(meshAABB);
		}

		//quick and dirty fix for skeletal animation:
		//considering skinned mesh is twice bigger than simple mesh.
		//TODO: implement frame based AABBs for skeletal animation 
		if(mSkeleton != NULL)
		{
			mAABB.grow(mAABB.getSize() * 0.5f);
		}
	}
	else
	{
		vec3 pos = getPosition();
		mAABB.setPoint(pos);
	}
}

}//namespace World { 

}//namespace Squirrel {
