#pragma once

#include <common/common.h>
#include <Resource/AnimationRunner.h>
#include <GUI/Container.h>
#include <GUI/Button.h>
#include <GUI/IntField.h>
#include "macros.h"

namespace Squirrel {
namespace Editor { 

struct AnimNodeRow
{
	AnimNodeRow(): animNode(NULL), nameEdit(NULL), firstFrameEdit(NULL)	{}
	Resource::AnimationNode *			animNode;
	GUI::Edit *							nameEdit;
	GUI::IntField *						firstFrameEdit;
	GUI::IntField *						lastFrameEdit;
	GUI::IntField *						repeatsEdit;
	GUI::Button *						playButton;
	GUI::Button *						deleteButton;
};

//TODO: rename to AnimRunnerInspector
class SQEDITOR_API AnimNodesInspector: 
	public GUI::ActionDelegate
{
	Resource::AnimationRunner * mInspectingObject;

	bool mChanged;

	GUI::Container * mTargetUIContainer;

	GUI::Button *	mAddAnimButton;
	GUI::IntField *	mFramesNumEdit;

	float mStartYPos;

	typedef std::list<AnimNodeRow> ANIM_NODES;

	ANIM_NODES mAnimNodes;
	bool mStretchContainer;

	void cleanUpNode(AnimNodeRow& node);
	void cleanUp();
	void fill();

	void fillNode(AnimNodeRow& node);

	AnimNodeRow * findNodeForAnim(Resource::AnimationNode * anim);

public://methods

	AnimNodesInspector();
	virtual ~AnimNodesInspector();

	bool isChanged() { 
		if(mChanged)
		{
			mChanged = false; 
			return true;
		}
		return false;
	}

	inline Resource::AnimationRunner * getInspectingObject() { return mInspectingObject; }

	void setInspectingObject(Resource::AnimationRunner * obj);

	void init(GUI::Container * targetUIContainer, float startYPos);
	void update();

	virtual bool processAction(const std::string& action, GUI::Element * sender);

//hide it from all excepting ReflectFieldDelegate ancestors
};

}//namespace Editor { 
}//namespace Squirrel {
