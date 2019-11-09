#include "Engine.h"
#include <FileSystem/Path.h>
#include <Render/Utils.h>
#include <GL/Render.h>
#include <World/Skeleton.h>
#include <Common/Settings.h>
#include <Audio/IAudio.h>

namespace Squirrel {
namespace Engine { 

int timeNodeUpdate = 0;
int timeNodeRender = 0;
int timeNodeRenderUI = 0;
int timeNodeFinish = 0;
int timeNodeOther = 0;

Resource::Program * mSimleColorProgram = NULL;
	
Engine::Engine() 
{
	bool forceCPUSkinning = Settings::Default()->getInt("Engine", "ForceCPUSkinning", 0) != 0;
	World::Skeleton::EnableCPUSkinning(forceCPUSkinning);
	
	//init GUI system
	GUI::Manager::Instance().init();
	
	//init GUI system
	mRenderManager = new RenderManager;
	mRenderManager->init();
	
	//init tmp benchmark
	timeNodeUpdate		= TimeCounter::Instance().addNode("update");
	timeNodeRender		= TimeCounter::Instance().addNode("render");
	timeNodeRenderUI	= TimeCounter::Instance().addNode("renderUI");
	timeNodeFinish		= TimeCounter::Instance().addNode("finish");
	timeNodeOther		= TimeCounter::Instance().addNode("other");
	
	mSimleColorProgram = Resource::ProgramStorage::Active()->add("GUI/GUI.glsl");
		
	Log::Instance().flush();
}

Engine::~Engine() 
{
}

void Engine::process(World::World * world)
{
	TimeCounter::Instance().setNodeTimeEnd(timeNodeOther);

	//end render previous frame

	TimeCounter::Instance().setNodeTimeBegin(timeNodeFinish);

	mRenderManager->end();

	TimeCounter::Instance().setNodeTimeEnd(timeNodeFinish);

	TimeCounter::Instance().calcTime();

	//update world

	TimeCounter::Instance().setNodeTimeBegin(timeNodeUpdate);

	float deltaTime = TimeCounter::Instance().getDeltaTime();

	GUI::Manager::Instance().update();

	world->updateRecursively(deltaTime);
	world->updateTransform();

	//get main camera
	Render::Camera * cam = Render::Camera::GetMainCamera();
	if(cam == NULL)
	{
		cam = new Render::Camera(Render::Camera::Perspective);
		cam->setPosition(Math::vec3(0, 0, -2.5f * world->getUnitsInMeter()));
		cam->setAsMain();
	}

	Audio::IAudio * audio = Audio::IAudio::GetActive();
	audio->setListenerPosition( cam->getPosition() );
	audio->setListenerOrientation( cam->getDirection(), vec3::AxisY() );

	//setup main camera

	Render::IRender * render = Render::IRender::GetActive();

	tuple2i screen = render->getWindow()->getSize();
	render->setViewport(0,0,screen.x,screen.y);

	float aspect = (float)screen.x / screen.y;
	cam->buildProjection(75 * DEG2RAD,aspect, 0.1f, world->getEffectiveViewDistance());
	render->setProjection(cam->getFinalMatrix());

	TimeCounter::Instance().setNodeTimeEnd(timeNodeUpdate);

	//start render new frame
	TimeCounter::Instance().setNodeTimeBegin(timeNodeRender);

	mRenderManager->begin();
	mRenderManager->render(world);

	Render::IProgram * colorProgram = mSimleColorProgram->getRenderProgram("TEXTURE;");

	colorProgram->bind();

	mRenderManager->draw3DDebugInfo();

	TimeCounter::Instance().setNodeTimeEnd(timeNodeRender);
	TimeCounter::Instance().setNodeTimeBegin(timeNodeRenderUI);

	//render UI

	Render::Utils::Begin2D();
	
	//if optimized
	//render->enableDepthTest(false);
	//else
	render->clear(false, true);
	
	mRenderManager->draw2DDebugInfo(render->getWindow()->getSize().x, render->getWindow()->getSize().y);
	
	GUI::Manager::Instance().render();

	//render statistics

	colorProgram->bind();

	render->setColor( vec4(1,1,1,1) );

	render->getUniformsPool().fetchUniforms(colorProgram);

	GUI::Font * mainFont = GUI::Manager::Instance().getMainFont();

	char strBuffer[128];

	//TimeCounter::Node * node = TimeCounter::Instance().getNode(TimeCounter::Instance().findNode("renderUI"));
	//sprintf(strBuffer, "FPS: %1.2f; %s: %1.3fms", TimeCounter::Instance().getFramesPerSecond(), node->name.c_str(), node->ms );
	//Framework::WindowManager::SetWindowText(strBuffer);

	sprintf(strBuffer, "FPS: %1.2f",  TimeCounter::Instance().getFramesPerSecond());
	mainFont->drawText(4, 6, strBuffer);
	
	sprintf(strBuffer, "delta: %1.2fms", deltaTime * 1000 );
	mainFont->drawText(4, 20, strBuffer);
	const int strOffset = 14;
	int yPos = 34;
	for(int i = 0; i < TimeCounter::Instance().getNodesNum(); ++i)
	{
		TimeCounter::Node * node = TimeCounter::Instance().getNode(i);
		sprintf(strBuffer, "%s: %1.4fms", node->name.c_str(), node->ms );
		mainFont->drawText(4, yPos += strOffset, strBuffer);
	}
	sprintf(strBuffer, "drawCalls: %d", render->getRenderStatistics().mDrawCallsNum );
	mainFont->drawText(4, yPos += strOffset, strBuffer);
	sprintf(strBuffer, "batches: %d", render->getRenderStatistics().mBatchesNum );
	mainFont->drawText(4, yPos += strOffset, strBuffer);
	sprintf(strBuffer, "tris: %d", render->getRenderStatistics().mTrianglesNum );
	mainFont->drawText(4, yPos += strOffset, strBuffer);

	sprintf(strBuffer, "cam: %1.2f, %1.2f, %1.2f", cam->getPosition().x, cam->getPosition().y, cam->getPosition().z );
	mainFont->drawText(4, yPos += strOffset, strBuffer);

	/*
	sprintf(strBuffer, "texture switches: %d", render->getRenderStatistics().mTextureSwitchesNum );
	mainFont->drawText(4, yPos += strOffset, strBuffer);
	sprintf(strBuffer, "state switches: %d", render->getRenderStatistics().mStateSwitchesNum );
	mainFont->drawText(4, yPos += strOffset, strBuffer);
	*/

	mainFont->flush();

	Render::Utils::End2D();

	TimeCounter::Instance().setNodeTimeEnd(timeNodeRenderUI);

	TimeCounter::Instance().setNodeTimeBegin(timeNodeOther);

	Input::Get()->update();

	render->setProjection(cam->getFinalMatrix());
}


}//namespace Engine { 
}//namespace Squirrel {
