#include "ObjectPanelController.h"
#include <Render/IRender.h>
#include <GUI/Manager.h>
#include <GUI/Button.h>
#include <GUI/Label.h>
#include <GUI/Edit.h>
#include <GUI/Switch.h>
#include <GUI/FloatField.h>
#include <Reflection/XMLSerializer.h>
#include <Reflection/XMLDeserializer.h>

namespace Squirrel {

using namespace World;
using namespace Reflection;

namespace Editor {

ObjectPanelController::ObjectPanelController()
{
	mInspectingObject	= NULL;
	mContainer			= NULL;
	mInspector			= NULL;
	mWindow				= NULL;

	mAnimsContainer		= NULL;
	mAnimsInspector		= NULL;

	mUpdateInterval		= 0.5f;
	mTimeBeforeNextUpdate= mUpdateInterval;

	mRotationWrapper	= new AtomicWrapperImpl<float>(&mRotation.x, 3);
}

ObjectPanelController::~ObjectPanelController()
{
	for(BEHAVIOUR_NODES_LIST::iterator it = mBehaviourNodes.begin(); it != mBehaviourNodes.end(); ++it)
	{
		dettachNode(&(*it));
	}

	mBehaviourNodes.clear();

	mWindow->del(mAnimsContainer);
	DELETE_PTR(mAnimsInspector);
	DELETE_PTR(mAnimsContainer);

	DELETE_PTR(mRotationWrapper);

	mWindow->del(mContainer);
	DELETE_PTR(mInspector);
	DELETE_PTR(mContainer);

	GUI::Manager::Instance().getMainPanel().delPanel( mWindow );
	DELETE_PTR(mWindow);
}

void ObjectPanelController::setRotation(vec3 rotation)
{
	mRotation = rotation;

	vec3 rotDelta = (mRotation - mPrevRotation) * DEG2RAD;
	mRotation = mPrevRotation;

	quat quatRot = mInspectingObject->getLocalRotation();

	if(fabsf(rotDelta.x) > EPSILON)
	{
		quat delta = quat().fromAxisAngle(vec3::AxisX(), rotDelta.x);
		quatRot = quatRot & delta;
	}
	if(fabsf(rotDelta.y) > EPSILON)
	{
		quat delta = quat().fromAxisAngle(vec3::AxisY(), rotDelta.y);
		quatRot = quatRot & delta;
	}
	if(fabsf(rotDelta.z) > EPSILON)
	{
		quat delta = quat().fromAxisAngle(vec3::AxisZ(), rotDelta.z);
		quatRot = quatRot & delta;
	}

	mInspectingObject->setLocalRotation(quatRot);
}

bool ObjectPanelController::valueChanged(ReflectFieldNode& node, int index)
{
	if(node.member->name != "Rotation")
		return false;

	GUI::FloatField * floatField = static_cast<GUI::FloatField *>(node.edits[index]);

	mRotation[index] = floatField->getValue();

	if(floatField->getLastValueChangeWay() == GUI::FloatField::EDITED)
	{
		quat rotX = quat().fromAxisAngle(vec3::AxisX(), mRotation.x * DEG2RAD);
		quat rotY = quat().fromAxisAngle(vec3::AxisY(), mRotation.y * DEG2RAD);
		quat rotZ = quat().fromAxisAngle(vec3::AxisZ(), mRotation.z * DEG2RAD);
		mInspectingObject->setLocalRotation(rotZ & rotY & rotX);
	}
	else if(floatField->getLastValueChangeWay() == GUI::FloatField::SCROLLED)
	{
		setRotation(mRotation);
	}

	return true;
}

void ObjectPanelController::fetchRotation()
{
	Math::quat quatRot = mInspectingObject->getLocalRotation();
	mRotation = quatRot.toPhysicAngle(mRotation) * RAD2DEG;
	mPrevRotation = mRotation;
}

bool ObjectPanelController::fetchToUI(ReflectFieldNode& node, tuple2i& fieldSize, ReflectObjectInspector * inspector)
{
	if(node.member->name != "Rotation")
		return false;

	fieldSize = inspector->fillAtomField(node, mRotationWrapper);

	return true;
}

void ObjectPanelController::init()
{
	mWindow = new GUI::Window();
	mWindow->setName("SceneObjectPanel");
	mWindow->setText("Scene Object Inspector");
	mWindow->setPosSep(255, 5);
	mWindow->setSizeSep(230, 350);
	mWindow->setActionDelegate(this);
	GUI::Manager::Instance().getMainPanel().addPanel( mWindow );

	//mObjInspector.init(mObjectPanel, 3);
}

void ObjectPanelController::dettachNode(BehaviourReflection * node)
{
	mWindow->del(node->separator);
	mWindow->del(node->header);
	mWindow->del(node->container);
}

void ObjectPanelController::setInspectingObject(World::SceneObject * obj)
{
	mInspectingObject = obj;

	std::set<Behaviour *> behaviourSet;

	if(mInspectingObject == NULL)
	{
		mWindow->setText("Scene Object Inspector");

		if(mContainer != NULL)
		{
			//mContainer->setVisible(false);
		}

		if(mInspector != NULL)
		{
			mInspector->setInspectingObject(NULL);
		}

		if(mAnimsInspector != NULL)
		{
			mAnimsInspector->setInspectingObject(NULL);
		}
	}
	else
	{
		mWindow->setText(mInspectingObject->getClassName() + " Inspector");

		if(mContainer == NULL)
		{
			mContainer = new GUI::Container();
			mContainer->setName("SceneObjectContainer");
			mContainer->setMargin(tuple4i(0,0,0,0));
			mContainer->setDrawStyle(0);
			mWindow->add(mContainer);
		}

		mContainer->setVisible(true);

		if(mInspector == NULL)
		{
			mInspector = new ReflectObjectInspector();
			mInspector->init(mContainer, 0);
			mInspector->setReflectFieldDelegete(this);
		}

		mInspector->setInspectingObject(mInspectingObject);

		if(mInspectingObject->getAnimations() != NULL)
		{
			if(mAnimsContainer == NULL)
			{
				//create container
				mAnimsContainer = new GUI::Container();
				mAnimsContainer->setName("AnimsContainer");
				mAnimsContainer->setMargin(tuple4i(0,0,0,0));
				mAnimsContainer->setSizeSep(1, 1);
				mAnimsContainer->setDrawStyle(0);
				mWindow->add(mAnimsContainer);
			}

			if(mAnimsInspector == NULL)
			{
				mAnimsInspector = new AnimNodesInspector();
				mAnimsInspector->init(mAnimsContainer, 0);
			}

			mAnimsInspector->setInspectingObject(mInspectingObject->getAnimations());
		}
		else if(mAnimsInspector != NULL)
		{
			mAnimsInspector->setInspectingObject(NULL);
		}

		behaviourSet.insert(mInspectingObject->getBehaviours()->begin(), mInspectingObject->getBehaviours()->end());
	}

	//create behaviours nodes

	std::set<Behaviour *> existedNodeSet;

	//remove exceed nodes and fill existedNodeSet
	BEHAVIOUR_NODES_LIST::iterator itNode = mBehaviourNodes.begin();
	while(itNode != mBehaviourNodes.end())
	{
		if(behaviourSet.find(itNode->behaviour) == behaviourSet.end())
		{
			//remove node
			dettachNode(&(*itNode));
			itNode = mBehaviourNodes.erase(itNode);
		}
		else
		{
			existedNodeSet.insert(itNode->behaviour);
			++itNode;
		}
	}

	if(mInspectingObject != NULL)
	{

		//add inexisted nodes
		SceneObject::BEHAVIOUR_LIST::iterator itBeh;
		for(itBeh = mInspectingObject->getBehaviours()->begin(); itBeh != mInspectingObject->getBehaviours()->end(); ++itBeh)
		{
			if(existedNodeSet.find((*itBeh)) == existedNodeSet.end())
			{
				//add behaviour node

				mBehaviourNodes.push_back(BehaviourReflection());
				BehaviourReflection& node = mBehaviourNodes.back();

				//set behaviour
				node.behaviour = (*itBeh);

				//create separator
				node.separator = new GUI::Separator();
				node.separator->setName("Separator");
				node.separator->setSizeSep(0, 1);
				mWindow->add(node.separator);

				//create header
				node.header = new GUI::Foldout();
				node.header->setName("BehaviourNodeHeader");
				node.header->setText(node.behaviour->getClassName());
				node.header->setSizeSep(0, 14);
				node.header->setActionDelegate(this);
				mWindow->add(node.header);

				//create container
				node.container = new GUI::Container();
				node.container->setName("BehaviourContainer");
				node.container->setMargin(tuple4i(0,0,0,0));
				node.container->setSizeSep(1, 1);
				node.container->setDrawStyle(0);
				mWindow->add(node.container);

				//create inspector
				node.inspector = new ReflectObjectInspector();
				node.inspector->init(node.container, 0);
				node.inspector->setInspectingObject(node.behaviour);
			}
		}
	}
	
	//update
	updateUI();
}

void ObjectPanelController::update(float deltaTime)
{
	mTimeBeforeNextUpdate -= deltaTime;

	if(mTimeBeforeNextUpdate <= 0)
	{
		updateUI();

		if(mInspectingObject != NULL && mAnimsInspector != NULL && mAnimsInspector->isChanged())
		{
			mInspectingObject->saveSubAnims();
		}

		mTimeBeforeNextUpdate = mUpdateInterval;
	}
}

void ObjectPanelController::updateUI()
{
	if(mInspectingObject == NULL) return;

	fetchRotation();

	int contentWidth = mWindow->getSize().x - mWindow->getMargin().x - mWindow->getMargin().z;

	const float ySpace = 2;
	float yPos = ySpace;

	mContainer->setPosSep(0, ySpace);
	mContainer->setSizeSep(contentWidth, 0);
	mInspector->update();
	yPos += mContainer->getSize().y;
	yPos += ySpace;

	if(mInspectingObject->getAnimations() != NULL)
	{
		mAnimsContainer->setPosSep(0, yPos);
		mAnimsContainer->setSizeSep(contentWidth, 0);
		mAnimsInspector->update();
		yPos += mAnimsContainer->getSize().y;
		yPos += ySpace;
	}

	for(BEHAVIOUR_NODES_LIST::iterator itNode = mBehaviourNodes.begin(); itNode != mBehaviourNodes.end(); ++itNode)
	{
		itNode->separator->setPosSep(0, yPos);
		itNode->separator->setSizeSep(contentWidth, itNode->separator->getSize().y);
		yPos += itNode->separator->getSize().y;
		yPos += ySpace;

		itNode->header->setPosSep(0, yPos);
		itNode->header->setSizeSep(contentWidth, itNode->header->getSize().y);
		yPos += itNode->header->getSize().y;
		yPos += ySpace;

		if(itNode->expanded)
		{
			itNode->container->setPosSep(0, yPos);
			itNode->container->setSizeSep(contentWidth, 0);
			itNode->inspector->update();
			yPos += itNode->container->getSize().y;
			yPos += ySpace;
		}
	}

	mWindow->updateScrolls();
}

bool ObjectPanelController::processAction(const std::string& action, GUI::Element * sender)
{
	if(sender == mWindow && action == GUI::Element::SIZE_CHANGED_ACTION)
	{
		updateUI();
		return true;
	}

	if(mInspectingObject == NULL) return false;

	for(BEHAVIOUR_NODES_LIST::iterator itNode = mBehaviourNodes.begin(); itNode != mBehaviourNodes.end(); ++itNode)
	{
		if(sender == itNode->header && action == GUI::Element::STATE_CHANGED_ACTION)
		{
			itNode->expanded = sender->getState() == GUI::Element::stateActive;
			if(itNode->expanded)
			{
				itNode->inspector->setInspectingObject(itNode->behaviour);
			}
			else
			{
				itNode->inspector->setInspectingObject(NULL);
			}
			updateUI();
			return true;
		}
	}

	return false;
}

}//namespace Render { 
}//namespace Squirrel {

