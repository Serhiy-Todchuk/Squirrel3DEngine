#pragma once

#include "BaseImporter.h"
#include <Common/common.h>

namespace Squirrel {
namespace DAEImport {

typedef std::vector<UniqueId> UniqueIdVec;
typedef std::map<UniqueId, UniqueIdVec> UniqueIdUniqueIdsMap;

class SQDAEIMPORTER_API Node
{
public:

	struct Transform
	{
		enum Type
		{
			None,
			Matrix,
			Rotate,
			Scale,
			Translate,
			Skew,
			Lookat
		};

		Type				type;
		UniqueId			sid;
		std::vector<float>	data;

		~Transform()
		{
			data.clear();
		}
	};

	typedef std::list<Node *>				NODES_LIST;
	typedef std::vector<Node *>				NODES_VECTOR;
	typedef std::map<UniqueId, Node * >		NODES_MAP;
	typedef std::pair<UniqueId, UniqueId>	MAT2PRIM_BIND;
	typedef std::map<UniqueId, Transform>	TRANSFORMS_MAP;

	Node(): mJoint(false), mParentIndex(-1) {};

	~Node()
	{
		mTransforms.clear();
		NODES_MAP::iterator it;
		for(it = mChildNodes.begin(); it != mChildNodes.end(); ++it)
		{
			DELETE_PTR(it->second);
		}
	};

	Math::mat4 bakeTransforms();
	
	bool isNotEmpty();
	bool removeEmptyNodes();

	UniqueId					mId;
	UniqueId					mSubId;

	bool						mJoint;

	TRANSFORMS_MAP				mTransforms;

	UniqueId					mMeshID;
	std::list<MAT2PRIM_BIND>	mMaterial2PrimitiveBindings;
	UniqueId					mControllerID;
	UniqueId					mSkeletonID;

	NODES_MAP					mChildNodes;

	//helper member for skeleton importing, used only if node is joint
	int							mParentIndex;
};

class SQDAEIMPORTER_API VisualSceneImporter: public BaseImporter
{
public:

private:

	Node::NODES_MAP		mRootNodes;
	std::list<UniqueId>	mSkeletonsIds;

public:
	VisualSceneImporter();
	~VisualSceneImporter(void);

public:

	/** 
	* Import the current visual scene with all scene nodes and transforms. 
	*/
	virtual void importObject(pugi::xml_node node);

	Node::NODES_MAP * getRootNodes();

	void removeEmptyNodes();

private:
    /*
    * Imports the data of the current node.
    */
	void importNode ( pugi::xml_node node,
					  Node* targetNode );

	void readMaterialBindings ( pugi::xml_node instanceNode,
								Node* targetNode );
};

}//namespace DAEImport { 
}//namespace Squirrel {