#pragma once

#include <common/common.h>
#include <GUI/Element.h>
#include <GUI/Panel.h>
#include <GUI/Label.h>
#include <World/World.h>
#include <Resource/Mesh.h>
#include <Resource/Program.h>
#include "ObjectPanelController.h"
#include "ResourcesBrowser.h"
#include "SceneBrowser.h"
#include "SceneInjection.h"
#include "ModelImporter.h"
#include "ProgressPanel.h"
#include <memory>
#include <map>
#include "macros.h"

namespace Squirrel {
namespace Editor { 

class SQEDITOR_API Editor: 
	public GUI::ActionDelegate,
	public ResourceOpenDelegate,
	public SceneBrowserDelegate
{
	enum AXIS
	{
		AXIS_X = 0,
		AXIS_Y = 1,
		AXIS_Z = 2,
		AXES_COUNT = 3
	};

	enum TRANSFORM_TYPES
	{
		TFORM_NONE		= -1,
		TFORM_MOVE		= 0,
		TFORM_ROTATE	= 1,
		TFORM_SCALE		= 2,
	};

	struct GizmoAxis
	{
		Math::vec4 color;
		Math::mat4 transform;

		Math::vec3 axis;

		Math::vec3 dragPositionScr;
		Math::vec3 dragDirectionScr;

		float dragCoef;

		Math::quat dragRotation;
		Math::vec3 dragPosition;
		Math::vec3 dragDirection;
	};

	GizmoAxis mAxes[AXES_COUNT];

	AXIS				mHighlightedAxis;
	TRANSFORM_TYPES		mHighlightedTform;
	bool				mLeftClicked;
	bool				mMiddleClicked;
	bool				mRightClicked;

	tuple2i				mPrevMousePos;

	bool mEnabled;

	std::auto_ptr<ObjectPanelController> mObjectPanelController;
	std::auto_ptr<ResourcesBrowser> mResourcesBrowser;
	std::auto_ptr<SceneBrowser> mSceneBrowser;
	std::auto_ptr<ProgressPanel> mProgressPanel;

	World::World * mProcessingWorld;
	World::SceneObject * mSelectedObject;

	std::auto_ptr<Resource::Mesh> mWiredBoxMesh;
	std::auto_ptr<Resource::Mesh> mArrowMesh;
	std::auto_ptr<Resource::Mesh> mRingMesh;

	Resource::Program * mGizmosProgram;

	Math::vec3 mDebugSpherePos;

	//dragging members
	Math::vec3	mMouseBeforeDrag;
	bool		mDragging;

	Plane		mDragPlane;
	tuple3<bool> mDragPosition;
	tuple3<bool> mDragAngles;
	tuple3<bool> mDragScale;

	std::map<std::string, ModelImporter *> mModelImporters;

	 ModelImporter * mCurrentModelImporter;

public://ctor/dtor
	Editor();
	~Editor();

public://methods

	void enable(bool enabled = true);
	bool isEnabled() { return mEnabled; }

	ObjectPanelController		* getObjectPanelController()	{ return mObjectPanelController.get(); }
	ResourcesBrowser	* getResourcesBrowser()	{ return mResourcesBrowser.get(); }

	void init(World::World * world);
	void process();

private:

	void renderBox(World::SceneObject * object);
	void renderObjectGizmos();
	void highlightObjectMoveArrows();
	void highlightObjectRotateRings(float scaleFactor);
	float updateObjectGizmosTransforms();
	void updateObjectGizmos();
	void processInput();
	void updateCamera();

	void focusOnSelected();
	float getObjectSize();

	bool raycast(World::SceneObjectsContainer * target, World::RaycastHit &out);
	bool projectMouseOnDragPlane(vec3 &out);

	void startDragging();
	void stopDragging();

	virtual bool processAction(const std::string& action, GUI::Element * sender);

	//
	virtual void resourceOpened(const FileSystem::FileInfo& resourceFile);

	//SceneBrowserDelegate
	virtual void didSelectObject(World::SceneObject *object);
	virtual void didDoubleclickObject(World::SceneObject *object);

	bool importModel(const std::string& filename);
};

}//namespace Editor { 
}//namespace Squirrel {
