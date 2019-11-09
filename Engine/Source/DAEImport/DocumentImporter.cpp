#include "DocumentImporter.h"

#include <fstream>
#include <time.h>

#include <Common/Settings.h>
#include <Common/Log.h>

#define AllTracksIntoOneAnim	true

#define COLLADA_TAG	"COLLADA"

#define LIBRARY_TAG	"library"
#define ASSET_TAG	"asset"

#define LIBRARY_IMAGES_TAG			"library_images"
#define LIBRARY_EFFECTS_TAG			"library_effects"
#define LIBRARY_MATERIALS_TAG		"library_materials"
#define LIBRARY_GEOMETRIES_TAG		"library_geometries"
#define LIBRARY_ANIMATIONS_TAG		"library_animations"
#define LIBRARY_CONTROLLERS_TAG		"library_controllers"
#define LIBRARY_VISUALSCENES_TAG	"library_visual_scenes"

namespace Squirrel {
namespace DAEImport { 

DocumentImporter::DocumentImporter(Render::IRender * render, Resource::TextureStorage * texStorage, Resource::MaterialLibrary * matLib):
	Resource::IModelImporter(render, texStorage, matLib), mModel(NULL)
{

}

DocumentImporter::~DocumentImporter(void)
{
}

const char_t * DocumentImporter::getImportingStageDesc() const 
{
	return mImportingStage.c_str();
}

void DocumentImporter::setImportStage(const char_t * str)
{
	mImportingStage = str;
}

bool DocumentImporter::parseXMLNode(pugi::xml_node node)
{
	const char * nodeName = node.name();
	
	if(strcmp(nodeName, ASSET_TAG) == 0)
	{
		pugi::xml_node unitNode = node.child("unit");
		if(unitNode)
			BaseImporter::mMetersInUnit = unitNode.attribute("meter").as_float();
		else
			BaseImporter::mMetersInUnit = 1.0f;

		pugi::xml_node upAxisNode = node.child("up_axis");
		std::string upAxisValue = upAxisNode.first_child().value();
		BaseImporter::mZIsUp = upAxisValue == "Z_UP";
	}
	else
	{
		std::map<std::string, BaseImporter *>::iterator it = mImporters.find(nodeName);

		if(it == mImporters.end()) 
			return false;

		BaseImporter * importer = it->second;

		char_t str[256];
		int counter = 0;

		//import library content
		for(pugi::xml_node objNode = node.first_child(); objNode; objNode = objNode.next_sibling())
		{
			sprintf(str, "Importing %s %d...", objNode.name(), counter++);

			setImportStage(str);

			importer->importObject(objNode);
		}
	}

	return true;
}

void DocumentImporter::load(pugi::xml_document * doc, const std::string& modelName)
{	
	setImportStage("Import started...");

	mModel = new Resource::Model;
	mModel->setName(modelName);

	mMeshImporter			= new MeshImporter(mRender);
	mMaterialImporter		= new MaterialImporter();
	mVisualSceneImporter	= new VisualSceneImporter();
	mAnimImporter			= new AnimationImporter();
	mCtrlImporter			= new ControllerImporter();

	mImporters.clear();
	mImporters[LIBRARY_IMAGES_TAG]			= mMaterialImporter;
	mImporters[LIBRARY_EFFECTS_TAG]			= mMaterialImporter;
	mImporters[LIBRARY_MATERIALS_TAG]		= mMaterialImporter;
	mImporters[LIBRARY_GEOMETRIES_TAG]		= mMeshImporter;
	mImporters[LIBRARY_ANIMATIONS_TAG]		= mAnimImporter;
	mImporters[LIBRARY_CONTROLLERS_TAG]		= mCtrlImporter;
	mImporters[LIBRARY_VISUALSCENES_TAG]	= mVisualSceneImporter;

	pugi::xml_node colladaRootNode = doc->child(COLLADA_TAG);

	//go through libraries
	for(mCurrentNode = colladaRootNode.first_child(); mCurrentNode; mCurrentNode = mCurrentNode.next_sibling())
	{
		parseXMLNode(mCurrentNode);
	}

	finish();

	delete mMeshImporter;
	delete mMaterialImporter;
	delete mVisualSceneImporter;
	delete mAnimImporter;
	delete mCtrlImporter;
}

bool DocumentImporter::load(const char_t * importFileName)
{	
	std::auto_ptr<pugi::xml_document> xmlDoc( new pugi::xml_document );
	if(xmlDoc.get() == NULL) 
		return false; 

	pugi::xml_parse_result result = xmlDoc->load_file(importFileName);

	if(result.status != pugi::status_ok)
	{
		Log::Instance().error("Parsing COLLADA XML-document", result.description());
		return false;
	}

	load(xmlDoc.get(), importFileName);

	return result.status == pugi::status_ok;
}

bool DocumentImporter::load(Data * data)
{	
	std::auto_ptr<pugi::xml_document> xmlDoc( new pugi::xml_document );
	if(xmlDoc.get() == NULL) 
		return false; 

	pugi::xml_parse_result result = xmlDoc->load_buffer(data->getData(), data->getLength());

	if(result.status != pugi::status_ok)
	{
		Log::Instance().error("Parsing COLLADA XML-document", result.description());
		return false;
	}

	load(xmlDoc.get(), data->getFileName());

	return result.status == pugi::status_ok;
}

Resource::AnimationTrack * DocumentImporter::bakeNodeAnims(Node * node, bool convertMatrices, float timeStep, size_t& aproxNumberOfFrames)
{
	int i = 0;

	//TODO: if(timeStep == 0) then don't bake, just copy tracks

	const AnimationImporter::CHANNELS_LIST * channelsList = mAnimImporter->getChannelsListForNode(node->mId);

	//Node is not animated
	if(channelsList == NULL)
		return NULL;

	if(channelsList->size() == 0)
		return NULL;

	AnimationImporter::CHANNELS_LIST::const_iterator itAnim;

	//calculate animation time range

	float startTime	= +999999;
	float endTime	= -999999;

	aproxNumberOfFrames = 0;

	for(itAnim = channelsList->begin(); itAnim != channelsList->end(); ++itAnim)
	{
		Resource::AnimationTrack * track = itAnim->animTrack;

		//update time range

		if(track->getTimeline()->size() > aproxNumberOfFrames)
			aproxNumberOfFrames = track->getTimeline()->size();

		if(track->getTimeline()->front() < startTime)
			startTime = track->getTimeline()->front();

		if(track->getTimeline()->back() > endTime)
			endTime = track->getTimeline()->back();

		//fix target indices

		int transformMember = itAnim->targetMember;

		int startIndex = transformMember >= 0 ? transformMember : 0;
		size_t endIndex = startIndex + track->getTargetComponentsNum();
		int arrIndex = 0;

		for(i = startIndex; i < endIndex; ++i)
		{
			(*track->getTargetIndices())[arrIndex++] = i;
		}
	}

	float animLength = endTime - startTime;
	int framesNum = Math::roundToBigger( animLength / timeStep );
	float actualTimeStep = animLength / framesNum;

	//create resulting track
	AnimationTrack * outTrack = new AnimationTrack(NULL, 16, framesNum + 1);
	outTrack->setInterpolationType(AnimationTrack::STEP);

	//animate target and iterativelly dump matrix
	for(i = 0; i <= framesNum; ++i)
	{
		float time = startTime + i * actualTimeStep;

		for(itAnim = channelsList->begin(); itAnim != channelsList->end(); ++itAnim)
		{
			Resource::AnimationTrack * track = itAnim->animTrack;

			const UniqueId& transformId = itAnim->targetSubId;

			Node::TRANSFORMS_MAP::const_iterator itTransform = node->mTransforms.find(transformId);
			if(itTransform == node->mTransforms.end())
			{
				continue;
			}
			const Node::Transform& transform = itTransform->second;

			track->updateBuffer(time, const_cast<float *>(&transform.data[0]), NULL);
		}

		Math::mat4 matrix = node->bakeTransforms();

		if(convertMatrices)
			matrix = BaseImporter::convertMatrix( matrix );

		(*outTrack->getTimeline())[i] = time;
		memcpy((*outTrack->getFramesData())[i], &matrix.x.x, sizeof(Math::mat4));
	}

	//fix target indices
	for(i = 0; i < 16; ++i)
	{
		(*outTrack->getTargetIndices())[i] = i;
	}

	return outTrack;
}

SkinController * DocumentImporter::importNodeSkin(Node * node, Resource::Model::Node * modelNode)
{
	SkinController * skinController = mCtrlImporter->getSkinController( node->mControllerID );
	ASSERT( skinController != NULL );
	if(skinController == NULL) return NULL;

	//get mesh id
	node->mMeshID = skinController->targetId;

	//get skin

	Resource::Skin * skin = NULL;
	
	std::map<UniqueId, _ID>::iterator itImportedSkin = mImportedSkinsIds.find( node->mControllerID );
	if(itImportedSkin == mImportedSkinsIds.end())
	{
		skin = skinController->skin;

		ASSERT( skin != NULL );

		mModel->getSkins()->push_back(skin);
		modelNode->mSkin = skin;
		modelNode->mSkinId = mModel->getSkins()->size() - 1;

		//fix skin joints with original (DAE) vertex position mapping
		MeshImporter::VERTEX_POSITION_MAP * vertexPositionIndices = mMeshImporter->getVertexMapping( node->mMeshID );
		if(vertexPositionIndices != NULL)
		{
			Resource::Skin::JointsBuffer originalJoints( skin->joints );
			skin->joints.setCount( vertexPositionIndices->size() );
			for(uint32 i = 0; i < vertexPositionIndices->size(); ++i)
			{
				int originalIndex = vertexPositionIndices->operator []( i );
				skin->joints.getData()[ i ] = originalJoints.getData()[ originalIndex ];
			}
		}

		mImportedSkinsIds[ node->mControllerID ] = modelNode->mSkinId;
	}
	else
	{
		skin = mModel->getSkins()->operator[]( itImportedSkin->second );
		modelNode->mSkin = skin;
		modelNode->mSkinId = itImportedSkin->second;
	}

	return skinController;
}

void DocumentImporter::importNode(Node * node, Resource::Model::Node * parentLink)
{
	ASSERT(node);
	if(!node) return;

	const char_t * nodeName =  NULL;
	if(node->mJoint && node->mSubId.length() > 0)
	{
		nodeName = node->mSubId.c_str();
	}
	else
	{
		nodeName = node->mId.c_str();
	}

	char_t str[128];
	sprintf(str, "Importing node %s...", nodeName);
	setImportStage(str);

	bool bakeAnimations = Settings::Default()->getInt("DAEImport", "BakeAnimations", 1) != 0;
	float bakedAnimationFramerate = Settings::Default()->getFloat("DAEImport", "BakedAnimationFramerate", 24.0f);
	float bakedAnimationTimestep = 1.0f / bakedAnimationFramerate;

	//extract transform

	Math::mat4 nodeTransform = node->bakeTransforms();

	//extract animation

	size_t aproxNumberOfFrames = 0;
	AnimationTrack * nodeAnim = bakeNodeAnims(node, false /*parentLink == NULL*/, bakeAnimations ? bakedAnimationTimestep : 0, aproxNumberOfFrames);
	
	//create and add modelNode
	Resource::Model::Node* modelNode = mModel->addNode(parentLink, nodeAnim != NULL);

	modelNode->mTransform = nodeTransform;

	modelNode->mName = nodeName;

	if(nodeAnim != NULL)
	{
		//get or create animation
		Resource::Animation * anim = mModel->createAnimation("main");

		anim->setFramesNum(aproxNumberOfFrames);

		//store anim track
		anim->addTrack(modelNode->mID, nodeAnim);
	}

	SkinController * skinController = NULL;
		
	//extract skin
	if( node->mControllerID.length() > 0 )
	{
		skinController = importNodeSkin(node, modelNode);
	}

	//extract mesh
	if(node->mMeshID.size() > 0 && node->mMaterial2PrimitiveBindings.size() > 0)
	{
		//now supported only one primitive per Node/modelNode
		//TODO: add support of more ones
		std::list<Node::MAT2PRIM_BIND>::iterator itMat2Prim = node->mMaterial2PrimitiveBindings.begin();

		UniqueId matId;
		UniqueId primId;

		while(itMat2Prim != node->mMaterial2PrimitiveBindings.end())
		{
			matId	= itMat2Prim->first;
			primId	= itMat2Prim->second;
			++itMat2Prim;

			Resource::Mesh * mesh =  mMeshImporter->getMesh( node->mMeshID, primId );

			//case when mesh importer does not support some geom type (i.e. spline)
			//or document has weird structure (i.e. 3DS Max FBX COLLADA exporter)
			if(mesh == NULL)
				continue;

			//apply bind-shape transfromations to skinned meshes
			if(skinController != NULL)
			{
				Resource::VertexBuffer * vb = mesh->getVertexBuffer();
				mSkinnedVBs[vb] = skinController;
			}

			modelNode->mMatLinks.push_back(Resource::Model::MaterialLink());
			Resource::Model::MaterialLink& matLink = modelNode->mMatLinks.back();

			matLink.mMesh = mesh;
			mModel->getMeshes()->push_back( mesh );
			matLink.mMeshId = mModel->getMeshes()->size() - 1;

			if(matId.length() > 0)
			{
				if(mModel->getLocalMaterials() == NULL)
				{
					mModel->setLocalMaterials(new Resource::MaterialLibrary());
				}

				char_t str[128];
				sprintf(str, "Importing textures for node %s...", nodeName);
				setImportStage(str);
				
				//attach material
				if(	!mMaterialImporter->getMaterialForId(
						matId, 
						mTexturesStorage, 
						mModel->getLocalMaterials(), 
						matLink )
					)
				{
					//TODO: warning
				}
			}
			
		}

	}

	//recursively import children
	Node::NODES_MAP::iterator it = node->mChildNodes.begin();
	for(; it != node->mChildNodes.end(); ++it)
	{
		importNode(it->second, modelNode);
	}
}

void DocumentImporter::applyBindTransform(Resource::VertexBuffer * vb, SkinController * skinController)
{
	ASSERT( vb );
	ASSERT( skinController );

	Resource::Skin * skin = skinController->skin;
	ASSERT( skin );

	for(int i = 0; i < skin->joints.getCount(); ++i)
	{
		Resource::Skin::Joint& joint = skin->joints.getData()[ i ];

		bool hasTan = vb->hasComponent(Resource::VertexBuffer::vcTangentBinormal);

		float binormalMultiplier;
		Math::vec3 srcTan;
		Math::vec3 srcPos = vb->getComponent<Resource::VertexBuffer::vcPosition>( i );
		Math::vec3 srcNor = vb->getComponent<Resource::VertexBuffer::vcNormal>( i );
		if(hasTan)
		{
			Math::vec4 tanBin = vb->getComponent<Resource::VertexBuffer::vcTangentBinormal>( i );
			srcTan = tanBin.getVec3();
			binormalMultiplier = tanBin.w;
		}

		Math::vec3 tan(0,0,0);
		Math::vec3 pos(0,0,0);
		Math::vec3 nor(0,0,0);

		for(int j = 0; j < joint.getCount(); ++j)
		{
			const tuple2i& boneInfluence = joint.getData()[ j ];
			float weight = skin->weights.getData()[ boneInfluence.x ];
			const Math::mat4& bindTransform = skinController->invBindMatrices.getData()[ boneInfluence.y ];
			const Math::mat3& bindRotation = bindTransform.getMat3();
			
			pos += (bindTransform * srcPos) * weight;
			nor += (bindRotation * srcNor)  * weight;
			if(hasTan) tan += (bindRotation * srcTan)  * weight;
		}

		vb->setComponent<Resource::VertexBuffer::vcPosition>( i,	pos );
		vb->setComponent<Resource::VertexBuffer::vcNormal>( i,	nor.normalized() );
		if(hasTan) vb->setComponent<Resource::VertexBuffer::vcTangentBinormal>( i,	
			Math::vec4(tan.normalized(), binormalMultiplier) );
	}
}

void DocumentImporter::finish()
{
	setImportStage("Finishing import...");

	mAnimImporter->sortChannelsByTargetId();

	Math::mat4 modelTransform = Math::mat4::Identity();
	if(BaseImporter::mZIsUp)
	{
		modelTransform = BaseImporter::convertMatrix( modelTransform );
	}
	mModel->setTransform(modelTransform);

	//clean up unused nodes
	mVisualSceneImporter->removeEmptyNodes();

	Node::NODES_MAP::iterator itFwNode = mVisualSceneImporter->getRootNodes()->begin();
	for(; itFwNode != mVisualSceneImporter->getRootNodes()->end(); ++itFwNode)
	{
		importNode( itFwNode->second, NULL );
	}

	(*mModel->getSharedVBs()) = (*mMeshImporter->getSharedVBs());

	setImportStage("Prepearing model...");

	mModel->calcBoundingVolume();
	mModel->prepareForGPUSkinning(4, true);
	mModel->generateTangentAndBinormal();

	setImportStage("Prepearing skins...");

	//apply bind-shape transfromations to skinned meshes
	std::map<Resource::VertexBuffer *, SkinController *>::iterator itSkinnedVB = mSkinnedVBs.begin();
	for(; itSkinnedVB != mSkinnedVBs.end(); ++itSkinnedVB)
	{
		applyBindTransform( itSkinnedVB->first, itSkinnedVB->second );
	}

	//remove imported VBs from MeshImporter to avoid their destruction
	mMeshImporter->getSharedVBs()->clear();

	//remove imported skins from ControllerImporter to avoid their destruction
	std::map<UniqueId, _ID>::iterator itImportedSkin = mImportedSkinsIds.begin();
	for(itImportedSkin; itImportedSkin != mImportedSkinsIds.end(); ++itImportedSkin)
	{
		SkinController * skinController = mCtrlImporter->getSkinController( itImportedSkin->first );
		skinController->skin = NULL;
	}

	//move local materials to global material library
	for(int i = 0; i < mModel->getLocalMaterials()->getCount(); ++i)
	{
		Resource::Material * mat = mModel->getLocalMaterials()->get(i);
		if(mat != NULL)
		{
			Resource::MaterialLibrary::Active()->add(mat);
		}
	}

	mImportedSkinsIds.clear();
	mSkinnedVBs.clear();

	setImportStage("");
}

}//namespace DAEImport { 
}//namespace Squirrel {