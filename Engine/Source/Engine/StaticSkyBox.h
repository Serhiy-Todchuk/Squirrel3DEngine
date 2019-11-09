#pragma once

#include <Common/common.h>
#include <Common/Log.h>
#include <Common/TimeCounter.h>
#include <Render/IRenderable.h>
#include <Resource/ResourceManager.h>
#include <Resource/Mesh.h>
#include "macros.h"

namespace Squirrel {
namespace Engine { 

class SQENGINE_API StaticSkyBox:
	public Render::IRenderable
{
	Resource::Texture *	mTexture;
	Resource::Mesh *	mMesh;
	Resource::Program *	mProgram;
	Math::mat3			mRotation;

	void init();

public:
	StaticSkyBox();
	~StaticSkyBox();

	void setRotation(const Math::mat3& rot) { mRotation = rot; }

	void init(const std::string& cubeTexture);
	void init(Resource::Texture * texture);

	virtual void render(Render::IRender * render, Render::Camera * camera);
};

}//namespace Engine { 
}//namespace Squirrel {