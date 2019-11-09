#pragma once

#include <Common/common.h>
#include <Common/Log.h>
#include <Common/TimeCounter.h>
#include <FileSystem/Path.h>
#include <Render/IRender.h>
#include <Resource/ResourceManager.h>
#include <Resource/AnimationRunner.h>
#include <World/World.h>
#include <World/Terrain.h>
#include <Render/Camera.h>
#include <Common/Input.h>
#include <GUI/Manager.h>
#include "RenderManager.h"

namespace Squirrel {
namespace Engine { 

//TODO: rename class to Pipeline

class SQENGINE_API Engine
{
	RenderManager * mRenderManager;

public:
	Engine();
	~Engine();

	void process(World::World * world);
};

}//namespace Engine { 
}//namespace Squirrel {