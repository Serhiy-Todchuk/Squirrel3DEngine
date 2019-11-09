#pragma once

#include <common/common.h>
#include <World/SceneObject.h>
#include <GUI/Window.h>
#include <GUI/Foldout.h>
#include <GUI/Separator.h>
#include <Reflection/AtomicWrapper.h>
#include <Reflection/Object.h>

#include "ReflectObjectInspector.h"
#include "AnimNodesInspector.h"
#include "macros.h"

#include <list>

namespace Squirrel {
namespace Editor { 

class SQEDITOR_API ObjectPanelController: 
	public GUI::ActionDelegate, public ReflectFieldDelegate
{
	struct BehaviourReflection
	{
		BehaviourReflection(): behaviour(NULL), header(NULL),
			separator(NULL), container(NULL), inspector(NULL), expanded(true) {}

		~BehaviourReflection()
		{
			DELETE_PTR(header);
			DELETE_PTR(separator);
			DELETE_PTR(inspector);
			DELETE_PTR(container);
		}

		World::Behaviour		* behaviour;
		GUI::Foldout			* header;
		GUI::Separator			* separator;
		GUI::Container			* container;
		ReflectObjectInspector	* inspector;
		bool					  expanded;
	};

	typedef std::list<BehaviourReflection> BEHAVIOUR_NODES_LIST;

	World::SceneObject		* mInspectingObject;
	GUI::Container			* mContainer;
	ReflectObjectInspector	* mInspector;
	GUI::Window				* mWindow;
	BEHAVIOUR_NODES_LIST	  mBehaviourNodes;

	GUI::Container			* mAnimsContainer;
	AnimNodesInspector		* mAnimsInspector;
	
	Reflection::AtomicWrapper * mRotationWrapper;
	vec3					  mRotation;
	vec3					  mPrevRotation;

	float mUpdateInterval;
	float mTimeBeforeNextUpdate;

	void dettachNode(BehaviourReflection * node);
	void fetchRotation();

	//implement ReflectFieldDelegate
	virtual bool valueChanged(ReflectFieldNode& node, int index);
	virtual bool fetchToUI(ReflectFieldNode& node, tuple2i& fieldSize, ReflectObjectInspector * inspector);

public://ctor/dtor
	ObjectPanelController();
	~ObjectPanelController();

public://methods

	vec3 getRotation() { return mRotation; }
	void setRotation(vec3 rotation);

	inline World::SceneObject * getInspectingObject() { return mInspectingObject; }

	void setInspectingObject(World::SceneObject * obj);

	void init();
	void updateUI();
	void update(float delta);

	//implement GUI::ActionDelegate
	virtual bool processAction(const std::string& action, GUI::Element * sender);
};

}//namespace Editor { 
}//namespace Squirrel {
