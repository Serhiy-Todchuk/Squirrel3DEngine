#pragma once

#include <Common/common.h>
#include <Common/Log.h>
#include <Common/TimeCounter.h>
#include <Render/IRenderable.h>
#include <Resource/ResourceManager.h>
#include <Resource/Mesh.h>
#include "SceneObject.h"

namespace Squirrel {
namespace World {

class SQWORLD_API Water:
	public SceneObject
{
	std::vector<Resource::Texture *>	mTextures;
	Resource::Mesh *	mMesh;

	void init();

public:
	Water();
	virtual ~Water();

	void init(const std::string& texFileName);

	virtual void render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info);

protected:

	virtual void calcAABB();
};

}//namespace World { 
}//namespace Squirrel {