#include "AnimNodesInspector.h"
#include <Render/IRender.h>
#include <GUI/Manager.h>
#include <GUI/Panel.h>
#include <GUI/Button.h>
#include <GUI/Label.h>
#include <GUI/Edit.h>
#include <GUI/Switch.h>
#include <GUI/IntField.h>
#include <GUI/FloatField.h>
#include <GUI/List.h>
#include <Common/TimeCounter.h>
#include <Reflection/XMLSerializer.h>
#include <Reflection/XMLDeserializer.h>
#include <limits>

namespace Squirrel {
namespace Editor {

using namespace Resource;

#ifdef min
# undef min
#endif
#ifdef max
# undef max
#endif

AnimNodesInspector::AnimNodesInspector()
{
	mInspectingObject = NULL;

	mTargetUIContainer = NULL;

	mAddAnimButton = NULL;
	mFramesNumEdit = NULL;

	mStartYPos = 0;

	mStretchContainer = true;

	mChanged = false;
}

AnimNodesInspector::~AnimNodesInspector()
{
	cleanUp();// :)
}

void AnimNodesInspector::init(GUI::Container * targetUIContainer, float startYPos)
{
	mTargetUIContainer = targetUIContainer;
	mStartYPos = startYPos;
}

void AnimNodesInspector::setInspectingObject(Resource::AnimationRunner * obj)
{
	if(mInspectingObject != obj)
	{
		cleanUp();
		mInspectingObject = obj;
		fill();
	}
}

void AnimNodesInspector::fillNode(AnimNodeRow& node)
{
	bool create = node.firstFrameEdit == NULL;

	Animation * animation = node.animNode->getAnimation();

	//create if needed
	if(create)
	{
		const std::string& name = node.animNode->getName();

		node.nameEdit = mTargetUIContainer->add<GUI::Edit>(name.c_str(), 0, 0, 10, 10);
		node.nameEdit->setActionDelegate(this);

		node.firstFrameEdit = mTargetUIContainer->add<GUI::IntField>((name + "_firstFrameEdit").c_str(), 0, 0, 10, 10);
		node.firstFrameEdit->setValueRange( tuple2i(0, animation->getFramesNum()) );
		node.firstFrameEdit->setActionDelegate(this);

		node.lastFrameEdit = mTargetUIContainer->add<GUI::IntField>((name + "_lastFrameEdit").c_str(), 0, 0, 10, 10);
		node.lastFrameEdit->setValueRange( tuple2i(0, animation->getFramesNum()) );
		node.lastFrameEdit->setActionDelegate(this);

		node.repeatsEdit = mTargetUIContainer->add<GUI::IntField>((name + "_repeatsEdit").c_str(), 0, 0, 10, 10);
		node.repeatsEdit->setValueRange( tuple2i(0, 99999) );
		node.repeatsEdit->setActionDelegate(this);

		node.playButton = mTargetUIContainer->add<GUI::Button>((name + "_playButton").c_str(), 0, 0, 10, 10);
		node.playButton->setActionDelegate(this);

		node.deleteButton = mTargetUIContainer->add<GUI::Button>((name + "_deleteButton").c_str(), 0, 0, 10, 10);
		node.deleteButton->setText("X");
		node.deleteButton->setActionDelegate(this);
	}

	//TODO: read-only main anim node

	//update values

	node.nameEdit->setText( node.animNode->getName() );

	float frameLength = animation->getTimeRange().y / animation->getFramesNum();

	int firstFrame = round( node.animNode->getTimeRange().x / frameLength );
	int lastFrame = round( node.animNode->getTimeRange().y / frameLength );

	node.firstFrameEdit->setValue(firstFrame);
	node.lastFrameEdit->setValue(lastFrame);

	node.repeatsEdit->setValue( node.animNode->getRepeatsNum() );

	node.playButton->setText(node.animNode->isPlaying() ? "Stop" : "Play");
}

AnimNodeRow * AnimNodesInspector::findNodeForAnim(Resource::AnimationNode * anim)
{
	for(ANIM_NODES::iterator it = mAnimNodes.begin(); it != mAnimNodes.end(); ++it)
	{
		if(it->animNode == anim)
			return &(*it);
	}

	return NULL;
}

void AnimNodesInspector::fill()
{
	if(mInspectingObject == NULL) return;
	if(mInspectingObject->getAnimNodes().size() == 0) return;

	const float containerWidth = (float)mTargetUIContainer->getSize().x - mTargetUIContainer->getMargin().x;

	const float xSpace = 1;
	const float ySpace = 1;
	const float nextLineLeftMargin = 8;

	const float labelsColumnWidth = 0.3f * containerWidth;

	GUI::Font * normalFont = GUI::Render::Instance().getFont( GUI::Render::sizeNormal );
	const int controlHeight = normalFont->getCharHeight();

	const tuple2i minAndMaxWidthPerEdit(20, 200);

	float currY = mStartYPos;
	float currX = 0;

	if(mAddAnimButton == NULL)
	{
		mAddAnimButton = mTargetUIContainer->add<GUI::Button>("AddAnimButton", 0, 0, 10, 10);
		mAddAnimButton->setText("Add");
		mAddAnimButton->setActionDelegate(this);
	}

	if(mFramesNumEdit == NULL)
	{
		mFramesNumEdit = mTargetUIContainer->add<GUI::IntField>("FramesNumEdit", 0, 0, 10, 10);
		mFramesNumEdit->setValue(0);
		mFramesNumEdit->setActionDelegate(this);
	}

	if(mAnimNodes.size() > 0)
	{
		Animation * animation = mAnimNodes.begin()->animNode->getAnimation();
		mFramesNumEdit->setValue( animation->getFramesNum() );
	}

	float firstRowFieldWidth = (containerWidth - (xSpace * 3)) / 2;

	mAddAnimButton->setPosSep((int)currX, (int)currY);
	mAddAnimButton->setSizeSep((int)firstRowFieldWidth, (int)controlHeight);
	currX += firstRowFieldWidth + xSpace;

	mFramesNumEdit->setPosSep((int)currX, (int)currY);
	mFramesNumEdit->setSizeSep((int)firstRowFieldWidth, (int)controlHeight);
	currX = 0;

	currY += controlHeight + ySpace;

	const AnimationRunner::ANIMS_MAP& anims = mInspectingObject->getAnimNodes();
	AnimationRunner::ANIMS_MAP::const_iterator animIt;

	for(animIt = anims.begin(); animIt != anims.end(); ++animIt)
	{
		AnimationNode * anim = animIt->second;

		AnimNodeRow * node = findNodeForAnim(anim);

		if(node == NULL)
		{
			mAnimNodes.push_back(AnimNodeRow());
			node = &mAnimNodes.back();
			node->animNode = anim;
		}

		fillNode( *node );

		node->nameEdit->setPosSep((int)currX, (int)currY);
		node->nameEdit->setSizeSep((int)labelsColumnWidth, (int)controlHeight);

		currX = labelsColumnWidth + xSpace;

		const float playButtonWidth = 30;
		const float delButtonWidth = static_cast<float>(controlHeight);
		float buttonsWidth = playButtonWidth + xSpace + delButtonWidth + xSpace;

		const int editsNum = 2;

		const float repeatsWidth = 30;
		float widthForEdits = containerWidth - currX - buttonsWidth - repeatsWidth;
		float minEditWidth = minAndMaxWidthPerEdit.x + xSpace;
		float minEditsWidth = minEditWidth * editsNum;

		//if there is not enough space for edits then move them to next line
		if(widthForEdits < minEditsWidth)
		{
			currY += controlHeight + ySpace;
			currX = nextLineLeftMargin;
			widthForEdits = containerWidth - currX - buttonsWidth - repeatsWidth;
		}

		float widthPerEdit = (widthForEdits / editsNum) - xSpace;
		widthPerEdit = Math::minValue(widthPerEdit, (float)minAndMaxWidthPerEdit.y);

		node->firstFrameEdit->setPosSep((int)currX, (int)currY);
		node->firstFrameEdit->setSizeSep((int)widthPerEdit, (int)controlHeight);
		currX += widthPerEdit + xSpace;

		node->lastFrameEdit->setPosSep((int)currX, (int)currY);
		node->lastFrameEdit->setSizeSep((int)widthPerEdit, (int)controlHeight);
		currX += widthPerEdit + xSpace;

		node->repeatsEdit->setPosSep((int)currX, (int)currY);
		node->repeatsEdit->setSizeSep((int)repeatsWidth, (int)controlHeight);
		currX += repeatsWidth + xSpace;

		node->playButton->setPosSep((int)currX, (int)currY);
		node->playButton->setSizeSep((int)playButtonWidth, (int)controlHeight);
		currX += playButtonWidth + xSpace;

		node->deleteButton->setPosSep((int)currX, (int)currY);
		node->deleteButton->setSizeSep((int)delButtonWidth, (int)controlHeight);

		currY += controlHeight + ySpace;
		currX = 0;
	}

	tuple2i containerSize = mTargetUIContainer->getSize();
	containerSize.y = static_cast<int>(currY) + mTargetUIContainer->getMargin().y + mTargetUIContainer->getMargin().w;
	mTargetUIContainer->setSize(containerSize);
}

void AnimNodesInspector::cleanUpNode(AnimNodeRow& node)
{
	mTargetUIContainer->del(node.nameEdit);
	DELETE_PTR(node.nameEdit);
	mTargetUIContainer->del(node.firstFrameEdit);
	DELETE_PTR(node.firstFrameEdit);
	mTargetUIContainer->del(node.lastFrameEdit);
	DELETE_PTR(node.lastFrameEdit);
	mTargetUIContainer->del(node.repeatsEdit);
	DELETE_PTR(node.repeatsEdit);
	mTargetUIContainer->del(node.playButton);
	DELETE_PTR(node.playButton);
	mTargetUIContainer->del(node.deleteButton);
	DELETE_PTR(node.deleteButton);
}

void AnimNodesInspector::cleanUp()
{
	mTargetUIContainer->del(mAddAnimButton);
	DELETE_PTR(mAddAnimButton);
	mTargetUIContainer->del(mFramesNumEdit);
	DELETE_PTR(mFramesNumEdit);

	for(ANIM_NODES::iterator it = mAnimNodes.begin(); it != mAnimNodes.end(); ++it)
	{
		cleanUpNode( *it );
	}
	mAnimNodes.clear();
}

void AnimNodesInspector::update()
{
	if(mInspectingObject == NULL) return;

	//cleanUp();
	fill();
}

bool AnimNodesInspector::processAction(const std::string& action, GUI::Element * sender)
{
	if(mInspectingObject == NULL) return false;

	bool changed = false;

	if(sender == mFramesNumEdit && action == GUI::Element::VALUE_CHANGED_ACTION)
	{
		if(mAnimNodes.size() == 0)
			return false;

		Animation * animation = mAnimNodes.begin()->animNode->getAnimation();

		animation->setFramesNum( mFramesNumEdit->getValue() );

		changed = true;
	}

	if(sender == mAddAnimButton && action == GUI::Button::LEFT_CLICK_ACTION)
	{
		if(mAnimNodes.size() == 0)
			return false;

		Animation * animation = mAnimNodes.begin()->animNode->getAnimation();
		Animatable * animatable = mAnimNodes.begin()->animNode->getTarget();

		AnimationNode * animNode = new AnimationNode(animation);
		animNode->setTarget(animatable);

		char_t str[64];
		sprintf(str, "Rename Me! %d", TimeCounter::GetTicks());

		AnimationRunner::ANIMS_MAP::const_iterator animIt = mInspectingObject->getAnimNodes().find(str);
		if(animIt != mInspectingObject->getAnimNodes().end())
		{
			sprintf(str, "Rename Me! %d", TimeCounter::GetTicks() + 1);
		}

		mInspectingObject->addAnimNode(str, animNode);

		mChanged = true;
		changed = true;
	}

	if(changed)
	{
		return true;
	}

	ANIM_NODES::iterator nodeToDelete = mAnimNodes.end();

	for(ANIM_NODES::iterator nodeIt = mAnimNodes.begin(); nodeIt != mAnimNodes.end(); ++nodeIt)
	{
		AnimationNode * anim = nodeIt->animNode;

		float frameLength = anim->getAnimation()->getTimeRange().y / anim->getAnimation()->getFramesNum();

		if(action == GUI::Element::VALUE_CHANGED_ACTION)
		{
			//check changing name

			if(sender == nodeIt->nameEdit)
			{
				AnimationRunner::ANIMS_MAP::const_iterator animIt = mInspectingObject->getAnimNodes().find(nodeIt->animNode->getName());

				if(animIt == mInspectingObject->getAnimNodes().end())
					return false;

				const std::string& newName = nodeIt->nameEdit->getText();

				if(newName != anim->getName())
				{
					AnimationRunner::ANIMS_MAP::const_iterator otherAnimIt = mInspectingObject->getAnimNodes().find(newName);

					if(otherAnimIt != mInspectingObject->getAnimNodes().end())
					{
						nodeIt->nameEdit->setText( anim->getName() );
						return false;
					}

					mInspectingObject->getAnimNodes().erase(animIt);
					mInspectingObject->addAnimNode(newName, anim);

					mChanged = true;
					changed = true;
				}
			}

			//check changing first frame index

			if(sender == nodeIt->firstFrameEdit)
			{
				int newFrame = nodeIt->firstFrameEdit->getValue();

				Math::vec2 timeRange = anim->getTimeRange();
				timeRange.x = newFrame * frameLength;

				if(!(timeRange - anim->getTimeRange()).isConsiderZero(0.001f))
				{
					mChanged = true;
					
					anim->setTimeRange( timeRange );
				}

				changed = true;
			}

			//check changing last frame index

			if(sender == nodeIt->lastFrameEdit)
			{
				int newFrame = nodeIt->lastFrameEdit->getValue();

				Math::vec2 timeRange = anim->getTimeRange();
				timeRange.y = newFrame * frameLength;

				if(!(timeRange - anim->getTimeRange()).isConsiderZero(0.001f))
				{
					mChanged = true;
					
					anim->setTimeRange( timeRange );
				}

				changed = true;
			}

			//check changing repeats num

			if(sender == nodeIt->repeatsEdit)
			{
				int newRepeates = nodeIt->repeatsEdit->getValue();

				if(newRepeates != anim->getRepeatsNum())
				{
					anim->setRepeatsNum(newRepeates);
					mChanged = true;
					changed = true; 
				}
			}
		}

		//on play/stop button

		if(sender == nodeIt->playButton && action == GUI::Button::LEFT_CLICK_ACTION)
		{
			if(anim->isPlaying())
			{
				mInspectingObject->stop();
				nodeIt->playButton->setText("Play");
			}
			else
			{
				mInspectingObject->playAnimNode(anim->getName());
				nodeIt->playButton->setText("Stop");
			}
			changed = true;
		}

		//on delete button

		if(sender == nodeIt->deleteButton && action == GUI::Button::LEFT_CLICK_ACTION)
		{
			AnimationRunner::ANIMS_MAP::const_iterator animIt = mInspectingObject->getAnimNodes().find(nodeIt->animNode->getName());

			if(animIt == mInspectingObject->getAnimNodes().end())
				return false;

			if(mInspectingObject->getAnimNodes().size() == 1)
				return false;

			mInspectingObject->stop();

			mInspectingObject->getAnimNodes().erase(animIt);
			DELETE_PTR(anim);

			nodeToDelete = nodeIt;
			
			changed = true;
		}

		if(changed)	break;
	}

	if(nodeToDelete != mAnimNodes.end())
	{
		mChanged = true;
		cleanUpNode(*nodeToDelete);
		mAnimNodes.erase(nodeToDelete);
	}

	return changed;
}

}//namespace Render { 
}//namespace Squirrel {

