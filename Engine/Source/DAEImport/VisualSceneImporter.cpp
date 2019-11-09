#include "VisualSceneImporter.h"
#include "DocumentImporter.h"
#include "Arrays.h"
#include <Common/Log.h>

namespace Squirrel {
namespace DAEImport { 

#define NODE_TYPE_NODE				"NODE"
#define NODE_TYPE_JOINT				"JOINT"

#define NODE_TAG					"node"

#define MATRIX_TAG					"matrix"
#define TRANSLATE_TAG				"translate"
#define ROTATE_TAG					"rotate"
#define SCALE_TAG					"scale"

#define INSTANCE_CONTROLLER_TAG		"instance_controller"
#define INSTANCE_MATERIAL_TAG		"instance_material"
#define INSTANCE_GEOMETRY_TAG		"instance_geometry"
#define BIND_MATERIAL_TAG			"bind_material"
#define SKELETON_TAG				"skeleton"

#define SYMBOL_ATTR					"symbol"

/*
Resource::AnimatableResource::TARGET_COMPONENTS DefineTransformAnimType(COLLADAFW::Transformation *transform)
{
	COLLADAFW::Rotate * rotateTform = NULL;
	switch(transform->getTransformationType())
	{
	case COLLADAFW::Transformation::TRANSLATE:

		//which value (x, y or z) need to animate exactly will be defined later by animation class of animList
		return Resource::AnimatableResource::ANIM_TRANSLATE_XYZ;

		break;
	case COLLADAFW::Transformation::ROTATE:

		rotateTform = TYPE_CAST<COLLADAFW::Rotate *>(transform);
		if(rotateTform->getRotationAxis().x == 1)
		{
			return Resource::AnimatableResource::ANIM_ROTATE_X;	
		}
		else if(rotateTform->getRotationAxis().y == 1)
		{
			return Resource::AnimatableResource::ANIM_ROTATE_Y;	
		}
		else if(rotateTform->getRotationAxis().z == 1)
		{
			return Resource::AnimatableResource::ANIM_ROTATE_Z;	
		}

		break;
	case COLLADAFW::Transformation::SCALE:

		ASSERT(false); //TODO: implement scale, consider it will be FLOAT anim class
		return Resource::AnimatableResource::ANIM_SCALE_XYZ;

		break;
	case COLLADAFW::Transformation::MATRIX:

		return Resource::AnimatableResource::ANIM_MATRIX4X4;

		break;
	default:
		ASSERT(false); //skew and lookAt are not supported
	}
	return Resource::AnimatableResource::ANIM_MATRIX4X4;
}
*/

Math::mat4 Node::bakeTransforms()
{
	Math::mat4 accumulatedMatrix;
	accumulatedMatrix.identity();

	for(TRANSFORMS_MAP::iterator it = mTransforms.begin(); it != mTransforms.end(); ++it)
	{
		Math::mat4 transformMatrix;
		transformMatrix.identity();

		const Transform& transform = it->second;

		switch(transform.type)
		{
		case Transform::Matrix:
			transformMatrix.fromArray(&transform.data[0]);
			break;
		case Transform::Translate:
			transformMatrix.setTranslate(Math::vec3(transform.data[0], transform.data[1], transform.data[2]));
			break;
		case Transform::Rotate:
			{
				Math::quat rotation;
				rotation.fromAxisAngle(Math::vec3(transform.data[0], transform.data[1], transform.data[2]), transform.data[3] * DEG2RAD);
				transformMatrix.setMat3(rotation.toRotationMatrix());
			}
			break;
		case Transform::Scale:
			transformMatrix.setMat3(Math::mat3::Scale(Math::vec3(transform.data[0], transform.data[1], transform.data[2])));
			break;
		case Transform::Skew:
		case Transform::Lookat:
				//Not implemented
		case Transform::None:
				ASSERT(false);
		}

		accumulatedMatrix = transformMatrix * accumulatedMatrix;
	}

	return accumulatedMatrix;
}


VisualSceneImporter::VisualSceneImporter()
{

}

VisualSceneImporter::~VisualSceneImporter(void)
{
	//cleanup nodes
	for(Node::NODES_MAP::iterator itNode = mRootNodes.begin(); itNode != mRootNodes.end(); ++itNode)
	{
		DELETE_PTR(itNode->second);
	}
}

void VisualSceneImporter::importObject(pugi::xml_node visualSceneNode)
{
	// Iterate over the root nodes.
	for(pugi::xml_node node = visualSceneNode.first_child(); node; node = node.next_sibling())
	{
        // Import the root node and all child nodes.
        importNode ( node, 0 );
	}
}

void VisualSceneImporter::readMaterialBindings ( pugi::xml_node instanceNode, Node* targetNode )
{
	pugi::xml_node bindMatNode = instanceNode.child(BIND_MATERIAL_TAG);
	if(!bindMatNode) return;

	pugi::xml_node techNode = bindMatNode.child(TECHNIQUE_COMMON_TAG.c_str());
	if(!techNode) return;

	pugi::xml_node matInstanceNode = techNode.child(INSTANCE_MATERIAL_TAG);
	for(; matInstanceNode; matInstanceNode = matInstanceNode.next_sibling(INSTANCE_MATERIAL_TAG))
	{
		//read primitive id
		UniqueId primId = matInstanceNode.attribute(SYMBOL_ATTR).value();

		//read material id
		UniqueId matIdLink = matInstanceNode.attribute(TARGET_ATTR.c_str()).value();
		UniqueId matId = matIdLink.substr(1);//skip first character as it is '#'

		//store binding
		Node::MAT2PRIM_BIND binding(matId, primId);
		targetNode->mMaterial2PrimitiveBindings.push_back(binding);
	}
}

// -----------------------------------
void VisualSceneImporter::importNode ( pugi::xml_node node, Node* targetParentNode )
{
	//read attributes
	UniqueId id		= node.attribute(ID_ATTR.c_str()).value();
	UniqueId subId	= node.attribute(SUB_ID_ATTR.c_str()).value();
	String typeStr	= node.attribute(TYPE_ATTR.c_str()).value();

	bool isJoint = typeStr == NODE_TYPE_JOINT;//default is NODE_TYPE_NODE

	//create node
	Node * newNode = new Node;
	if(targetParentNode)
	{
		targetParentNode->mChildNodes[id] = newNode;
	}
	else
	{
		mRootNodes[id] = newNode;
	}

	//setup general node data
	newNode->mId = id;
	newNode->mJoint = isJoint;
	newNode->mSubId = subId;

	//read transformations
	for(pugi::xml_node tformNode = node.first_child(); tformNode; tformNode = tformNode.next_sibling())
	{
		Node::Transform::Type tformType = Node::Transform::None;

		int arraySize = 0;

		if(String(tformNode.name()) == MATRIX_TAG)
		{
			tformType = Node::Transform::Matrix;
			arraySize = 16;
		}
		else if(String(tformNode.name()) == TRANSLATE_TAG)
		{
			tformType = Node::Transform::Translate;
			arraySize = 3;
		}
		else if(String(tformNode.name()) == ROTATE_TAG)
		{
			tformType = Node::Transform::Rotate;
			arraySize = 4;
		}
		else if(String(tformNode.name()) == SCALE_TAG)
		{
			tformType = Node::Transform::Scale;
			arraySize = 3;
		}
		//TODO: skew and lookat

		if(tformType != Node::Transform::None)
		{
			UniqueId tformSubId		= tformNode.attribute(SUB_ID_ATTR.c_str()).value();

			FloatArray floatArr(arraySize, tformNode.child_value());

			//create and add transform
			Node::Transform& tform = newNode->mTransforms[tformSubId] = Node::Transform();

			tform.sid = tformSubId;
			tform.type = tformType;
			tform.data.insert(tform.data.begin(), floatArr.getArray(), floatArr.getArray() + arraySize);
		}
	}

	//read controller instance
	pugi::xml_node ctrlNode = node.child(INSTANCE_CONTROLLER_TAG);
	if(ctrlNode)
	{
		//read controller id
		UniqueId ctrlIdLink = ctrlNode.attribute(URL_ATTR.c_str()).value();
		ASSERT(ctrlIdLink.length() > 0);
		UniqueId ctrlId = ctrlIdLink.substr(1);//skip first character as it is '#'
		newNode->mControllerID = ctrlId;

		//read skeleton id
		pugi::xml_node skeletonNode = ctrlNode.child(SKELETON_TAG);
		if(skeletonNode)
		{
			UniqueId skeletonIdLink = skeletonNode.child_value();
			ASSERT(skeletonIdLink.length() > 0);
			UniqueId skeletonId = skeletonIdLink.substr(1);//skip first character as it is '#'
			newNode->mSkeletonID = skeletonId;

			mSkeletonsIds.push_back(skeletonId);
		}
		else
		{
			Log::Instance().warning("DAEImport: importing visual scene nodes:",
						"Controller instance has no skeleton node!"); 
		}

		readMaterialBindings(ctrlNode, newNode);
	}

	//read geometry instance
	pugi::xml_node geomNode = node.child(INSTANCE_GEOMETRY_TAG);
	if(geomNode)
	{
		//read geometry id
		UniqueId geomIdLink = geomNode.attribute(URL_ATTR.c_str()).value();
		UniqueId geomId = geomIdLink.substr(1);//skip first character as it is '#'
		newNode->mMeshID = geomId;

		readMaterialBindings(geomNode, newNode);
	}

    // Recursive call for all child elements.
	for(pugi::xml_node childNode = node.child(NODE_TAG); childNode; childNode = childNode.next_sibling(NODE_TAG))
    {
        importNode ( childNode, newNode );
    }
}

Node::NODES_MAP * VisualSceneImporter::getRootNodes() 
{ 
	return &mRootNodes; 
}

bool Node::isNotEmpty()
{
	if(mMeshID.size() > 0 || mControllerID.size() > 0 || mJoint) return true;

	NODES_MAP::iterator it = mChildNodes.begin();
	for(; it != mChildNodes.end(); ++it)
	{
		if(it->second->isNotEmpty())
		{
			return true;
		}
	}

	return false;
}

bool Node::removeEmptyNodes()
{
	if(!isNotEmpty())
	{
		return true;
	}
	else
	{
		NODES_MAP::iterator itChild = mChildNodes.begin();
		while(itChild != mChildNodes.end())
		{
			if(itChild->second->removeEmptyNodes())
			{
				DELETE_PTR(itChild->second);
				itChild = mChildNodes.erase(itChild);
			}
			else
			{
				++itChild;
			}
		}
	}
	return false;
}

void VisualSceneImporter::removeEmptyNodes()
{
	//remove empty nodes
	Node::NODES_MAP::iterator itChild = mRootNodes.begin();
	while(itChild != mRootNodes.end())
	{
		if(itChild->second->removeEmptyNodes())
		{
			DELETE_PTR(itChild->second);
			itChild = mRootNodes.erase(itChild);
		}
		else
		{
			++itChild;
		}
	}
}

}//namespace DAEImport { 
}//namespace Squirrel {