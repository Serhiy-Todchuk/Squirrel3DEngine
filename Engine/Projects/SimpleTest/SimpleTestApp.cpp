#include "SimpleTestApp.h"
#include <Common/Input.h>
#include <Common/Platform.h>
#include <World/Light.h>
#include <World/ParticleSystem.h>
#include <World/Body.h>
#include <World/Terrain.h>
#include <World/Water.h>
#include <Engine/DynamicSkySphere.h>
#include <Auxiliary/HeightMapGen.h>

#include "FPSWalker.h"
#include "Player.h"
#include "Automated.h"

//implement Application::Create method, so SimpleTestApp class instantiates at startup
Application * Squirrel::Auxiliary::Application::Create()
{
	return new SimpleTestApp;
}

const char_t * ExitDialogTitle = "Exit";

void SimpleTestApp::windowDialogEnd(int answer, void * contextInfo)
{
	if(contextInfo == ExitDialogTitle && answer == WindowDialogDelegate::answerYes)
	{
		Platform::TerminateProcess();
	}
}

void SimpleTestApp::process()
{
	if(Input::Get()->isKeyUp( Input::Escape ))
	{
		mWindow->showDialog(ExitDialogTitle, "Are you sure you want to quit?", true, this);
	}
	if(Input::Get()->isKeyUp( Input::Tab ))
	{
		mEditor->enable( !mEditor->isEnabled() );
	}
	if(Input::Get()->isKeyUp( Input::Pause ))
	{
		TimeCounter::Instance().pause( !TimeCounter::Instance().isPaused() );
	}
	
	EditorApp::process();
}

void SimpleTestApp::initWorld()
{
	mWorld = new World::World;
	
	mWorld->init(Settings::Default());
	
	//init sky
	
	World::Light * light = World::Light::Create(mWorld);
	light->setGlobal(true);
	light->setLightType(Render::Light::ltDirectional);
	light->setTransformLight(false);
	light->setName("Sky");
	light->setAmbient(tuple4ub(57, 57, 57, 255));
	light->getLight().mShadow = true;
	
	Engine::DynamicSkySphere * sky = light->addBehaviour<Engine::DynamicSkySphere>();
	sky->init("skyboxes/purpleNebulaStars2.sqtex");
	sky->addSun();
	//sky->setEnabled(false);
	sky->setLight(&light->getLight());
	mWorld->setSky(sky, false);
	
	//init FPS shooter
	
	World::SceneObject * fpShooter = new World::SceneObject();
	fpShooter->setName("FPShooter");
	fpShooter->setGlobal(true);
	mWorld->addSceneObject( fpShooter );
	
	FPSWalker * player1 = new FPSWalker;
	player1->setEnabled(false);
	fpShooter->addBehaviour( player1 );
	player1->initGun("f_plasma.jpg", "q3plasma.wav");
	
	//init spider
	
	Resource::Model * model = NULL;
	
	std::string path = "spider.sqmdl";
	std::string ext = FileSystem::Path::GetExtension( path );
	if(ext == "sqmdl")
	{
		model = Resource::ModelStorage::Active()->add( path );
	}
	
	World::Light * spotLight = World::Light::Create(mWorld);
	spotLight->setLightType(Render::Light::ltSpot);
	spotLight->setName("Spot");
	spotLight->setLocalPosition(vec3(0, 5, 0));
	spotLight->getLight().setDirection(vec3(0, -1.0f, 0));
	spotLight->setRadius(40.0f);
	spotLight->getLight().mShadow = true;
	
	if(model)
	{
		model->calcBoundingVolume();
		model->prepareForGPUSkinning(4, true);
		
		//add player
		World::Body * sceneObj = new World::Body();
		sceneObj->initWithModel( model );
		sceneObj->setName("Player");
		mWorld->addSceneObject( sceneObj );
		Player * player1 = new Player;
		sceneObj->addBehaviour( player1 );
		
		//add AI
		sceneObj = new World::Body();
		sceneObj->setLocalPosition(vec3(0, 0, 15));
		sceneObj->setLocalRotation(quat().fromAxisAngle(vec3::AxisX(), 90.0f));
		sceneObj->initWithModel( model );
		sceneObj->setName( "AISpider" );
		mWorld->addSceneObject( sceneObj );
		Automated * ai = new Automated;
		sceneObj->addBehaviour( ai );
	}
	
	//init terrain
	
	using RenderData::VertexBuffer;
	
	World::Terrain * terra = new World::Terrain();
	terra->initTextures("terrain/rock.jpg", "terrain/sand.jpg", "terrain/grass.jpg", "terrain/snow.jpg");
	terra->init(Settings::Default(), true);
	mWorld->setTerrain(terra);

	World::Water * water = new World::Water();
	water->setName("Water");
	water->init("waternormals/water**.png");
	mWorld->addSceneObject(water);

	mSaveSettings = true;
}
