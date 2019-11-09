#include "Editor.h"
#include "SceneInjection.h"

#include <GUI/Manager.h>
#include <GUI/Panel.h>
#include <GUI/Button.h>
#include <GUI/Label.h>
#include <GUI/Edit.h>
#include <GUI/Switch.h>
#include <GUI/FloatField.h>
#include <FileSystem/Path.h>
#include <Common/Settings.h>
#include <Common/TimeCounter.h>
#include <pugixml/pugixml.hpp>
#include <algorithm>
#include <locale>

#include <Render/IRender.h>

#include <Resource/ResourceManager.h>

#include <World/Body.h>

namespace Squirrel {
namespace Editor {

using namespace Math;
using namespace RenderData;

const int WIRED_BOX_MESH_VERTS		= 8;

const float GIZMO_LINE_LENGTH		= 1.0f;
const float GIZMO_RING_RADIUS		= 0.7f;
const float GIZMO_WIDTH_IN_PIXELS	= 6.0f;

const float OBJ_FOCUS_DISTANCE		= 3.0f;

const float OBJ_INSTANTIATION_DISTANCE		= 4.0f;//meters

const char_t * WINDOWS_LAYOUT_FILENAME = "editorWindowsLayout.xml";
const char_t * WINDOWS_LAYOUT_NODENAME = "layout";

Editor::Editor()
{
	mEnabled		= true;

	mDragging		= false;

	mLeftClicked	= false;
	mRightClicked	= false;
	mMiddleClicked	= false;

	mCurrentModelImporter = NULL;

	mHighlightedAxis = AXIS_X;
	mHighlightedTform = TFORM_NONE;

	//init axes

	mAxes[AXIS_X].color = vec4(1, 0, 0, 1);
	mAxes[AXIS_Y].color = vec4(0, 1, 0, 1);
	mAxes[AXIS_Z].color = vec4(0, 0, 1, 1);

	mAxes[AXIS_X].axis = vec3(1, 0, 0);
	mAxes[AXIS_Y].axis = vec3(0, 1, 0);
	mAxes[AXIS_Z].axis = vec3(0, 0, 1);

	//load gizmo shader

	Resource::ProgramStorage * programStorage = Resource::ProgramStorage::Active();

	mGizmosProgram = programStorage->add("gizmos.glsl");

	//init gizmo ring mesh

	Resource::TorusBuilder torusBuilder;
	torusBuilder.mVertType		= VT_PNT;
	torusBuilder.mSlices		= 8;
	torusBuilder.mStacks		= 40;
	torusBuilder.mRadBig		= GIZMO_RING_RADIUS;
	torusBuilder.mRadSmall		= 0.009f;
	mRingMesh.reset( torusBuilder.buildMesh() );

	Resource::CylinderBuilder cylinderBuilder;
	cylinderBuilder.mVertType	= VT_PNT;

	cylinderBuilder.mSlices		= 12;
	cylinderBuilder.mStacks		= 4;

	cylinderBuilder.mHeight		= 0.92f;
	cylinderBuilder.mRadTop		= 0.01f;
	cylinderBuilder.mRadBottom	= 0.01f;
	Resource::Mesh * cyl = cylinderBuilder.buildMesh();

	cylinderBuilder.mHeight		= 0.15f;
	cylinderBuilder.mRadTop		= 0.0f;
	cylinderBuilder.mRadBottom	= 0.034f;
	Resource::Mesh * arr = cylinderBuilder.buildMesh();

	mat4 tform;
	tform.identity();
	tform.setTranslate(vec3(0, 0.9f, 0));
	arr->applyTransform(tform);

	mArrowMesh.reset( Resource::Mesh::Combine(*cyl, *arr) );
	mArrowMesh->getIndexBuffer()->setPolyOri(IndexBuffer::poNone);
	DELETE_PTR(cyl);
	DELETE_PTR(arr);

	mArrowMesh->getVertexBuffer()->setStorageType(IBuffer::stGPUStaticMemory);
	mRingMesh->getVertexBuffer()->setStorageType(IBuffer::stGPUStaticMemory);

	//
	//create wired box rendering mesh
	//
	
	mWiredBoxMesh.reset( new Resource::Mesh() );

	//init vertex buffer
	VertexBuffer * vb = mWiredBoxMesh->createVertexBuffer(VCI2VT(VertexBuffer::vcPosition), WIRED_BOX_MESH_VERTS);
	vb->setStorageType(VertexBuffer::stGPUDynamicMemory);

	//init index buffer
	IndexBuffer * ib = mWiredBoxMesh->createIndexBuffer(24);
	ib->setPolyOri(IndexBuffer::poNone);
	ib->setPolyType(IndexBuffer::ptLines);

	int ii = 0;

	//
	ib->setIndex(ii++, 0);
	ib->setIndex(ii++, 1);

	ib->setIndex(ii++, 1);
	ib->setIndex(ii++, 2);

	ib->setIndex(ii++, 2);
	ib->setIndex(ii++, 3);

	ib->setIndex(ii++, 3);
	ib->setIndex(ii++, 0);

	//
	ib->setIndex(ii++, 4);
	ib->setIndex(ii++, 5);

	ib->setIndex(ii++, 5);
	ib->setIndex(ii++, 6);

	ib->setIndex(ii++, 6);
	ib->setIndex(ii++, 7);

	ib->setIndex(ii++, 7);
	ib->setIndex(ii++, 4);

	//
	ib->setIndex(ii++, 0);
	ib->setIndex(ii++, 4);

	ib->setIndex(ii++, 1);
	ib->setIndex(ii++, 5);

	ib->setIndex(ii++, 2);
	ib->setIndex(ii++, 6);

	ib->setIndex(ii++, 3);
	ib->setIndex(ii++, 7);
	
}

Editor::~Editor()
{
	//save main windows layout

	pugi::xml_document layoutDoc;
	for(int i = 0; i < GUI::Manager::Instance().getMainPanel().numPanel(); ++i)
	{
		GUI::Panel * panel = GUI::Manager::Instance().getMainPanel().getPanel(i);
		pugi::xml_node layoutNode = layoutDoc.append_child(WINDOWS_LAYOUT_NODENAME);
		layoutNode.append_attribute("name").set_value(panel->getName().c_str());
		layoutNode.append_attribute("posX").set_value(panel->getPos().x);
		layoutNode.append_attribute("posY").set_value(panel->getPos().y);
		layoutNode.append_attribute("sizeX").set_value(panel->getSize().x);
		layoutNode.append_attribute("sizeY").set_value(panel->getSize().y);
	}
	layoutDoc.save_file(WINDOWS_LAYOUT_FILENAME);

	if(mGizmosProgram)
		Resource::ProgramStorage::Active()->release( mGizmosProgram->getID() );

	for(std::map<std::string, ModelImporter *>::iterator it = mModelImporters.begin(); it != mModelImporters.end(); ++it)
	{
		DELETE_PTR(it->second);
	}
}

void Editor::enable(bool enabled)
{
	mEnabled = enabled;
	GUI::Manager::Instance().getMainPanel().setVisible(enabled);
}

void Editor::init(World::World * world)
{
	mProcessingWorld = world;
	mSelectedObject = NULL;
	if(world == NULL) return;

	//init GUI

	mObjectPanelController.reset( new ObjectPanelController );
	mObjectPanelController->init();

	mResourcesBrowser.reset( new ResourcesBrowser );
	Resource::ModelStorage * modelStorage = Resource::ModelStorage::Active();
	FileSystem::FileStorage * fileStorage = modelStorage->getContentSource();
	mResourcesBrowser->setResourceContainer( fileStorage );
	mResourcesBrowser->clearAllowedExtensions();
	mResourcesBrowser->addAllowedExtension("sqmdl");
	mResourcesBrowser->addAllowedExtension("dae");
	mResourcesBrowser->addAllowedExtension("fbx");
	mResourcesBrowser->setResourceOpenDelegate( this );
	mResourcesBrowser->show();

	mSceneBrowser.reset( new SceneBrowser);
	mSceneBrowser->setContentRoot( mProcessingWorld );
	mSceneBrowser->setSceneBrowserDelegate( this );
	mSceneBrowser->checkoutContent();
	mSceneBrowser->show();

	mProgressPanel.reset( new ProgressPanel );

	//restore main windows layout

	pugi::xml_document layoutDoc;
	pugi::xml_parse_result result = layoutDoc.load_file(WINDOWS_LAYOUT_FILENAME);
	if(result)
	{
		pugi::xml_node layoutNode = layoutDoc.child(WINDOWS_LAYOUT_NODENAME);
		for(; layoutNode; layoutNode = layoutNode.next_sibling(WINDOWS_LAYOUT_NODENAME))
		{
			std::string panelName = layoutNode.attribute("name").value();
			GUI::Panel * panel = GUI::Manager::Instance().getMainPanel().getPanel(panelName);
			if(panel != NULL)
			{
				int posX	= layoutNode.attribute("posX").as_int();
				int posY	= layoutNode.attribute("posY").as_int();
				int sizeX	= layoutNode.attribute("sizeX").as_int();
				int sizeY	= layoutNode.attribute("sizeY").as_int();

				panel->setPosSep(posX, posY);
				panel->setSizeSep(sizeX, sizeY);
			}
		}
	}
}

void Editor::startDragging()
{
	mDragging		= true;
}

void Editor::stopDragging()
{
	mDragging		= false;
	mDragPosition.x	= false;
	mDragPosition.y	= false;
	mDragPosition.z	= false;
}

bool Editor::raycast(World::SceneObjectsContainer * target, World::RaycastHit &out)
{
	Render::IRender * render = Render::IRender::GetActive();
	tuple2i mouseWin = Input::Get()->getMousePos();
	tuple4i viewport(0, 0,  render->getWindow()->getSize().x,  render->getWindow()->getSize().y);
	vec2 mouseScr((float)mouseWin.x, (float)viewport.w - mouseWin.y);
	Ray worldMouseRay = Render::Camera::GetMainCamera()->getRayFromScreenPoint(mouseScr, viewport);
	return target->findClosestIntersection(worldMouseRay, out);
}

bool Editor::projectMouseOnDragPlane(vec3 &out)
{
	Render::IRender * render = Render::IRender::GetActive();
	tuple2i mouseWin = Input::Get()->getMousePos();
	tuple4i viewport(0, 0,  render->getWindow()->getSize().x,  render->getWindow()->getSize().y);
	vec2 mouseScr((float)mouseWin.x, (float)viewport.w - mouseWin.y);
	Ray worldMouseRay = Render::Camera::GetMainCamera()->getRayFromScreenPoint(mouseScr, viewport);
	float outLen;
	if(mDragPlane.intersectByRay(worldMouseRay.mOrigin, worldMouseRay.mDirection, outLen))
	{
		out = worldMouseRay.mOrigin + worldMouseRay.mDirection * outLen;
		return true;
	}
	return false;
}

void Editor::renderBox(World::SceneObject * object)
{
	Render::IRender * render = Render::IRender::GetActive();

	AABB objBounds = object->getAABB();

	RenderData::VertexBuffer * vb = mWiredBoxMesh->getVertexBuffer();

	for(int i = 0; i < WIRED_BOX_MESH_VERTS; ++i)
	{
		vec3 vert = objBounds.getVertex(i);
		vb->setComponent<VertexBuffer::vcPosition>(i, vert);
	}

	vb->update(0, WIRED_BOX_MESH_VERTS * vb->getVertexSize());

	Render::IProgram * program = mGizmosProgram->getRenderProgram("LIGHTING;");
	program->bind();
	program->uniform("ambient", 1.0f );

	render->setTransform(Math::mat4().identity());
	render->setColor(vec4(0.5f, 1.0f, 0.5f, 1.0f));

	render->getUniformsPool().fetchUniforms(program);

	render->setupVertexBuffer(vb);
	render->renderIndexBuffer(mWiredBoxMesh->getIndexBuffer());
}

void Editor::renderObjectGizmos()
{
	Render::IRender * render = Render::IRender::GetActive();

	Render::IProgram * program = NULL;
	program = mGizmosProgram->getRenderProgram("LIGHTING;");

	const float ambient = 0.3f;

	program->bind();
	program->uniform("light", vec3(-1,-1,-1).normalized() );
	program->uniform("ambient", ambient );

	render->enableDepthTest(Render::IRender::depthTestOff);

	render->setupVertexBuffer(mArrowMesh->getVertexBuffer());

	for(int i = 0; i < AXES_COUNT; ++i)
	{
		render->setTransform(mAxes[i].transform);
		Math::vec4 color = mAxes[i].color;
		if(mHighlightedTform == TFORM_MOVE && mHighlightedAxis == static_cast<AXIS>(i))
		{
			color += Math::vec4(0.5f, 0.5f, 0.5f, 0.0f);
			program->uniform("ambient", 1.0f );
		}
		else
		{
			program->uniform("ambient", 0.3f );
		}
		render->setColor(color);

		render->getUniformsPool().fetchUniforms(program);
		render->renderIndexBuffer(mArrowMesh->getIndexBuffer());
	}

	render->setupVertexBuffer(mRingMesh->getVertexBuffer());

	for(int i = 0; i < AXES_COUNT; ++i)
	{
		render->setTransform(mAxes[i].transform);
		Math::vec4 color = mAxes[i].color;
		if(mHighlightedTform == TFORM_ROTATE && mHighlightedAxis == static_cast<AXIS>(i))
		{
			color += Math::vec4(0.5f, 0.5f, 0.5f, 0.0f);
			program->uniform("ambient", 1.0f );
		}
		else
		{
			program->uniform("ambient", 0.3f );
		}
		render->setColor(color);

		render->getUniformsPool().fetchUniforms(program);
		render->renderIndexBuffer(mRingMesh->getIndexBuffer());
	}

	render->enableDepthTest();
}

void Editor::highlightObjectMoveArrows()
{
	Render::IRender * render = Render::IRender::GetActive();

	tuple4i viewport(0, 0, render->getWindow()->getSize().x, render->getWindow()->getSize().y);
	tuple2i mouse = Input::Get()->getMousePos();//mouse in window space
	//mouse in screen space
	vec3 mouseScr((float)mouse.x, (float)viewport.w - mouse.y, 0);

	vec3 initGizmoCenter(0, 0, 0);
	vec3 initAxisDirection(0, 1, 0);
	vec3 initMoveLineEnd(initAxisDirection * GIZMO_LINE_LENGTH);

	float minDistanceFromScreen = FLT_MAX;

	//highlight move arrows

	for(int i = 0; i < AXES_COUNT; ++i)
	{
		vec3 moveStart		= mAxes[i].transform * initGizmoCenter;
		vec3 moveEnd		= mAxes[i].transform * initMoveLineEnd;

		//project start and end of arrow on screen
		vec3 moveStartScr	= Render::Camera::GetMainCamera()->project(moveStart, viewport); 
		vec3 moveEndScr		= Render::Camera::GetMainCamera()->project(moveEnd, viewport); 

		float distanceFromScreen = moveEndScr.z;

		moveStartScr.z	= 0;
		moveEndScr.z	= 0;

		//closest point on arrow to mouse (in screen space)
		vec3 closestPointScr = Math::closestPointOnLine(moveStartScr, moveEndScr, mouseScr);
		float distanceFromArrowToMouse = (mouseScr - closestPointScr).len();

		bool highlighted = distanceFromArrowToMouse <= GIZMO_WIDTH_IN_PIXELS;
		if(highlighted && distanceFromScreen < minDistanceFromScreen)
		{
			minDistanceFromScreen	= distanceFromScreen;
			mHighlightedTform		= TFORM_MOVE;
			mHighlightedAxis		= static_cast<AXIS>(i);

			vec3 dragDirWld		= moveStart - moveEnd;
			float dragDirLenWld	= dragDirWld.len();

			vec3 dragDirScr		= moveStartScr - moveEndScr;
			float dragDirLenScr	= dragDirScr.len();

			mAxes[i].dragCoef			= dragDirLenWld / dragDirLenScr;

			//world space
			mAxes[i].dragPosition		= moveStart;//mSelectedObject->getLocalPosition()
			mAxes[i].dragDirection		= dragDirWld / dragDirLenWld;

			//screen space
			mAxes[i].dragPositionScr	= closestPointScr;
			mAxes[i].dragDirectionScr	= dragDirScr / dragDirLenScr;
		}
	}
}

void Editor::highlightObjectRotateRings(float scaleFactor)
{
	Render::IRender * render = Render::IRender::GetActive();

	tuple4i viewport(0, 0, render->getWindow()->getSize().x, render->getWindow()->getSize().y);
	tuple2i mouse = Input::Get()->getMousePos();//mouse in window space
	//mouse in screen space
	vec3 mouseScr((float)mouse.x, (float)viewport.w - mouse.y, 0);

	vec3 initGizmoCenter(0, 0, 0);
	vec3 initAxisDirection(0, 1, 0);

	float minDistanceFromScreen = FLT_MAX;

	//highlight rotation rings

	for(int i = 0; i < AXES_COUNT; ++i)
	{
		Ray worldMouseRay = Render::Camera::GetMainCamera()->getRayFromScreenPoint(mouseScr.getVec2(), viewport);

		mat4 rotMarix = mAxes[i].transform.inverse().transposed();

		vec3 rotCenter		= mAxes[i].transform * initGizmoCenter;
		vec3 rotAxis		= rotMarix * initAxisDirection;
		rotAxis.normalize();

		float ringRadius = GIZMO_RING_RADIUS * scaleFactor;

		float t = 0;
		vec3 intersectionPoint(0,0,0);

		//vector to eye
		vec3 eyeVec = (Render::Camera::GetMainCamera()->getPosition() - rotCenter).normalized();

		Plane rotPlane(rotAxis, rotCenter);

		if(fabsf(eyeVec * rotAxis) > 0.02)
		{
			// in this case ring looks like an ellipse or a circle on a screen
			rotPlane.intersectByRay(worldMouseRay.mOrigin, worldMouseRay.mDirection, t);
			intersectionPoint = worldMouseRay.mOrigin + worldMouseRay.mDirection * t;
		}
		else
		{
			// in this case ring looks like a line on a screen

			//project mouse ray on sphere
			float sphereIntersection = Math::raySphereIntersection(
				worldMouseRay.mOrigin, worldMouseRay.mDirection, rotCenter, ringRadius);

			if(sphereIntersection < 0) continue;

			vec3 sphereInterPoint = worldMouseRay.mOrigin + worldMouseRay.mDirection * sphereIntersection;

			intersectionPoint = rotPlane.project(sphereInterPoint);
		}

		vec3 vectorToMouse = (intersectionPoint - rotCenter).normalized();
		vec3 closestPointOnRing = rotCenter + vectorToMouse * ringRadius;

		vec3 closestPointScr	= Render::Camera::GetMainCamera()->project(closestPointOnRing, viewport);

		float distanceFromScreen = closestPointScr.z;


		closestPointScr.z = 0;

		vec3 dir = mouseScr - closestPointScr;

		bool highlighted = dir.len() <= GIZMO_WIDTH_IN_PIXELS;
		if(highlighted && distanceFromScreen < minDistanceFromScreen)
		{
			minDistanceFromScreen	= distanceFromScreen;
			mHighlightedTform		= TFORM_ROTATE;
			mHighlightedAxis		= static_cast<AXIS>(i);

			//calculate radius of ring in screen space

			//drag direction
			vec3 dragDir = vec3(vectorToMouse.y, -vectorToMouse.x, 0); 

			//farest point on the ring, used for calculating screen space radius
			vec3 farestPointDir;

			if(fabsf((eyeVec * rotAxis) - 1) < EPSILON)
			{
				// if rotation axis is paralel to eye vector 
				// then doesn't matter what vector to choose for calculating radius 
				// as ring looks like a circle on a screen
				farestPointDir = dragDir;
			}
			else
			{
				// in this case ring looks like an ellipse or a line on a screen
				// taking perpendicular to axis of ring as a direction to farest point

				farestPointDir = eyeVec ^ rotAxis;
			}

			vec3 farestPoint = rotCenter + farestPointDir * ringRadius;
			vec3 dragDirPoint = rotCenter + dragDir * ringRadius;

			//project points on screen
			vec3 rotCenterScr	= Render::Camera::GetMainCamera()->project(rotCenter, viewport);
			vec3 dragScr		= Render::Camera::GetMainCamera()->project(dragDirPoint, viewport);
			vec3 farestScr		= Render::Camera::GetMainCamera()->project(farestPoint, viewport);

			rotCenterScr.z = 0;
			farestScr.z = 0;
			dragScr.z = 0;

			vec3 farestDirScr	= farestScr - rotCenterScr;
			float ringRadiusScr	= farestDirScr.len();

			mAxes[i].dragRotation = mSelectedObject->getLocalRotation();

			mAxes[i].dragCoef			= 90.0f / ringRadiusScr;

			//screen space
			mAxes[i].dragPositionScr	= closestPointScr;
			mAxes[i].dragDirectionScr	= -farestDirScr / ringRadiusScr;
		}
	}
}

float Editor::updateObjectGizmosTransforms()
{
	mat4 arrowsParentTform = mSelectedObject->getTransform();
	
	vec3 objScale = arrowsParentTform.extractScale();
	vec3 scale(1.0f/objScale.x, 1.0f/objScale.y, 1.0f/objScale.z);

	float distanceFromCam = (mSelectedObject->getPosition() - Render::Camera::GetMainCamera()->getPosition()).len();

	float scaleFactor = distanceFromCam * 0.3f;//max(mUnitsInMeter, objSize);
	scale *= scaleFactor;

	mat4 scaleTform = mat4::Transform(vec3::Zero(), mat3::Identity(), scale);
	arrowsParentTform = arrowsParentTform * scaleTform;

	mAxes[AXIS_X].transform = arrowsParentTform * mat4::RotZ(0, -1);//-90 around z
	mAxes[AXIS_Y].transform = arrowsParentTform;
	mAxes[AXIS_Z].transform = arrowsParentTform * mat4::RotX(0, 1);//90 around z

	return scaleFactor;
}

void Editor::updateObjectGizmos()
{
	Render::IRender * render = Render::IRender::GetActive();

	tuple4i viewport(0, 0, render->getWindow()->getSize().x, render->getWindow()->getSize().y);
	tuple2i mouse = Input::Get()->getMousePos();
	vec3 mouseScr((float)mouse.x, (float)viewport.w - mouse.y, 0);

	const float screenLineLength = 2000;

	if(mLeftClicked)
	{
		if(mHighlightedTform == TFORM_MOVE)
		{
			GizmoAxis& axis = mAxes[static_cast<int>(mHighlightedAxis)];

			vec3 startLineScr	= axis.dragPositionScr - axis.dragDirectionScr * screenLineLength;
			vec3 endLineScr		= axis.dragPositionScr + axis.dragDirectionScr * screenLineLength;

			vec3 projectionScr	= Math::closestPointOnLine(startLineScr, endLineScr, mouseScr);

			vec3 dragDirSrc = axis.dragPositionScr - projectionScr;
			float distanceSrc = dragDirSrc.len();

			if(distanceSrc < EPSILON) return;

			dragDirSrc /= distanceSrc;

			float distanceWld = distanceSrc * axis.dragCoef;

			float dragSign = - (dragDirSrc * axis.dragDirectionScr);//dot

			vec3 posOffset = axis.dragDirection * distanceWld * dragSign;
			
			mSelectedObject->setLocalPosition(axis.dragPosition + posOffset);

			mObjectPanelController->updateUI();
		}
		else if(mHighlightedTform == TFORM_ROTATE)
		{
			int axisIndex = static_cast<int>(mHighlightedAxis);
			GizmoAxis& axis = mAxes[axisIndex];

			vec3 startLineScr	= axis.dragPositionScr - axis.dragDirectionScr * screenLineLength;
			vec3 endLineScr		= axis.dragPositionScr + axis.dragDirectionScr * screenLineLength;

			vec3 projectionScr	= Math::closestPointOnLine(startLineScr, endLineScr, mouseScr);

			vec3 dragDirSrc = axis.dragPositionScr - projectionScr;
			float distanceSrc = dragDirSrc.len();

			if(distanceSrc < EPSILON) return;

			dragDirSrc /= distanceSrc;

			float distanceWld = distanceSrc * axis.dragCoef;

			float dragSign = - (dragDirSrc * axis.dragDirectionScr);//dot

			quat quatDelta = quat().fromAxisAngle(axis.axis, distanceWld * dragSign * DEG2RAD);
			mSelectedObject->setLocalRotation(axis.dragRotation & quatDelta);

			mObjectPanelController->updateUI();
		}
	}
}

void Editor::processInput()
{
	if(Input::Get()->isKeyUp(Input::Mouse0)) mLeftClicked	= false;
	if(Input::Get()->isKeyUp(Input::Mouse1)) mRightClicked	= false;
	if(Input::Get()->isKeyUp(Input::Mouse2)) mMiddleClicked	= false;
	if(Input::Get()->isKeyDown(Input::Mouse0) && !mLeftClicked)
	{
		if(mHighlightedTform == TFORM_MOVE || mHighlightedTform == TFORM_ROTATE)
		{
			mPrevMousePos = Input::Get()->getMousePos();
			mLeftClicked = true;
		}
	}
	if(Input::Get()->isKeyDown(Input::Mouse1))
	{
		mPrevMousePos = Input::Get()->getMousePos();
		mRightClicked = true;
	}
	if(Input::Get()->isKeyDown(Input::Mouse2))
	{
		mPrevMousePos = Input::Get()->getMousePos();
		mMiddleClicked = true;
	}
}

void Editor::updateCamera()
{
	if(mHighlightedTform != TFORM_NONE && mLeftClicked) return;

	tuple2i mouseWin		= Input::Get()->getMousePos();
	tuple2i mouseDeltaWin	= mouseWin - mPrevMousePos;
	mPrevMousePos			= mouseWin;

	const float turnStep	= 0.02f;
	const float moveStep	= mProcessingWorld->getUnitsInMeter() * 0.05f;
	const float forwardStep	= mProcessingWorld->getUnitsInMeter() * 0.3f;

	Render::Camera * cam = Render::Camera::GetMainCamera();

	if(mRightClicked)
	{
		if(mouseDeltaWin.x != 0)
		{
			quat rot = quat().fromAxisAngle(vec3::AxisY(), (float)mouseDeltaWin.x * -turnStep);

			vec3 center = cam->getPosition() + cam->getDirection() * forwardStep;

			vec3 dir = rot.apply(cam->getDirection());
			cam->setDirection(dir);

			vec3 pos = center - dir * forwardStep;
			cam->setPosition(pos);
		}
		if(mouseDeltaWin.y != 0)
		{
			vec3 axis = vec3::AxisY() ^ cam->getDirection();
			quat rot = quat().fromAxisAngle(axis, (float)mouseDeltaWin.y * turnStep);

			vec3 center = cam->getPosition() + cam->getDirection() * forwardStep;

			vec3 dir = rot.apply(cam->getDirection());
			cam->setDirection(dir);

			vec3 pos = center - dir * forwardStep;
			cam->setPosition(pos);
		}
	}
	else if(mMiddleClicked)
	{
		vec3 camDir = Render::Camera::GetMainCamera()->getDirection();
		vec3 axisY = vec3::AxisY();
		vec3 right = camDir ^ axisY;
		vec3 up = camDir ^ right;

		if(mouseDeltaWin.x != 0)
		{
			cam->setPosition( cam->getPosition() + right * moveStep * (float)mouseDeltaWin.x );
		}
		if(mouseDeltaWin.y != 0)
		{
			cam->setPosition( cam->getPosition() + up * moveStep * (float)mouseDeltaWin.y );
		}
	}

	float scroll = Input::Get()->getMouseScroll();
	if(fabsf(scroll) > EPSILON)
		cam->movePosDirXYZ(scroll * forwardStep);

	vec3 instPos = cam->getPosition() + cam->getDirection() * OBJ_INSTANTIATION_DISTANCE * mProcessingWorld->getUnitsInMeter();
	mSceneBrowser->setInstantiationPos(	instPos );
}

void Editor::process()
{
	if(mProcessingWorld == NULL) return;

	if(Render::Camera::GetMainCamera() == NULL) return;

	//
	//process input
	//

	processInput();

	updateCamera();

	if(mCurrentModelImporter && mCurrentModelImporter->isRunning())
	{
		mProgressPanel->setText(mCurrentModelImporter->getStageDesc());

		Resource::Model * model = mCurrentModelImporter->getModel();
		if(model != NULL)
		{
			std::string modelFileName =  FileSystem::Path::GetFileNameWithoutExtension(model->getName()) + ".sqmdl";
			Resource::ModelStorage::Active()->addNew(modelFileName, model);

			//add scene obj

			World::Body * sceneObj = new World::Body();
			sceneObj->initWithModel( model );
			mProcessingWorld->addSceneObject( sceneObj );
			mSceneBrowser->checkoutContent();

			mProgressPanel->close();

			mCurrentModelImporter = NULL;
		}
	}

	if(mSelectedObject != NULL)
	{
		mObjectPanelController->update(TimeCounter::Instance().getDeltaTime());

		//
		// update gizmos transforms
		//

		float scaleFactor = updateObjectGizmosTransforms();

		//
		// highlight gizmos
		//

		if(!mLeftClicked && !mRightClicked && !mMiddleClicked)
		{
			mHighlightedTform = TFORM_NONE;

			highlightObjectRotateRings(scaleFactor);

			if(mHighlightedTform == TFORM_NONE)
				highlightObjectMoveArrows();
		}
		
		//
		// update gizmos
		//

		updateObjectGizmos();

		//
		// render gizmos
		//

		renderObjectGizmos();

		//draw selected object AABB

		renderBox(mSelectedObject);

		//delete selected obj

		if(Input::Get()->isKeyUp(Input::Delete) || Input::Get()->isKeyUp(Input::Backspace))
		{
			mSceneBrowser->deleteSelected();
		}
	}
}

bool Editor::importModel(const std::string& filename)
{
	Render::IRender * render = Render::IRender::GetActive();

	Context * graphicsStreamingContext = render->getWindow()->getContexts()[1];

	std::string ext = FileSystem::Path::GetExtension(filename);

	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	std::map<std::string, ModelImporter *>::iterator it = mModelImporters.find(ext);

	mCurrentModelImporter = NULL;

	if(it != mModelImporters.end())
	{
		mCurrentModelImporter = it->second;
	}
	else
	{
		std::string module = ext;

		std::transform(module.begin(), module.end(), module.begin(), ::toupper);

		//combine module name from extensio
		module = std::string("Sq") + module + "Importer";

		mCurrentModelImporter = new ModelImporter(module.c_str(), render, Resource::TextureStorage::Active());

		if(!mCurrentModelImporter->isOk())
		{
			DELETE_PTR(mCurrentModelImporter);
			return false;
		}

		mModelImporters[ext] = mCurrentModelImporter;
	}

	if(mCurrentModelImporter->startImport(filename, graphicsStreamingContext))
	{
		mProgressPanel->show((std::string("Importing ") + filename + " model...").c_str());

		return true;
	}

	return false;
}

void Editor::resourceOpened(const FileSystem::FileInfo& resourceFile)
{
	Resource::Model * model = NULL;

	Resource::ModelStorage * modelStorage = Squirrel::Resource::ModelStorage::Active();

	if(resourceFile.extension == "sqmdl")
	{
		model = modelStorage->add( resourceFile.path );
	}
	else
	{
		importModel(resourceFile.absPath);
	}

	if(model)
	{
		//add scene obj

		World::Body * sceneObj = new World::Body();
		sceneObj->initWithModel( model );
		mProcessingWorld->addSceneObject( sceneObj );
		mSceneBrowser->checkoutContent();
	}
}

float Editor::getObjectSize()
{
	AABB objBounds = mSelectedObject->getAABB();

	if(objBounds.isEmpty())
	{
		return 0.0f;
	}

	float size = 0;
	vec3 sizes(
		fabsf(objBounds.max.x - objBounds.min.x),
		fabsf(objBounds.max.y - objBounds.min.y),
		fabsf(objBounds.max.z - objBounds.min.z)
		);
	if(sizes.x > size) size = sizes.x;
	if(sizes.y > size) size = sizes.y;
	if(sizes.z > size) size = sizes.z;

	return size;
}

void Editor::focusOnSelected()
{
	float minFocusDistance = OBJ_FOCUS_DISTANCE * mProcessingWorld->getUnitsInMeter();

	float focusDistance = minFocusDistance + getObjectSize();

	vec3 pos = mSelectedObject->getPosition();
	vec3 dir = Render::Camera::GetMainCamera()->getDirection();

	Render::Camera::GetMainCamera()->setPosition( pos + dir * -focusDistance );
}

void Editor::didSelectObject(World::SceneObject *object)
{
	mObjectPanelController->setInspectingObject(object);
	mSelectedObject = object;
}

void Editor::didDoubleclickObject(World::SceneObject *object)
{
	didSelectObject(object);
	focusOnSelected();
}

bool Editor::processAction(const std::string& action, GUI::Element * sender)
{
	return false;
}

}//namespace Render { 
}//namespace Squirrel {

