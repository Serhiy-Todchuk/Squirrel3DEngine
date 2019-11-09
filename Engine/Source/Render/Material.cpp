#include "Material.h"

namespace Squirrel {

namespace RenderData { 


Material::Material(void)
{
	mID			= -1;
	mName		= "unnamed";
	mLighting	= "default";
	mPhysics	= "default";

	mDiffuse	= Math::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpecular	= Math::vec4(0.5f, 0.5f, 0.5f, 0.5f);
	mEmission	= Math::vec4(0.0f, 0.0f, 0.0f, 0.1f);
	mAmbient	= Math::vec4(0.2f, 0.2f, 0.2f, 0.2f);
	mReflect	= Math::vec4(0.0f, 0.0f, 0.0f, 0.1f);
	mShininess	= 64.0f;
	mOpacity	= 1.0f;
}

Material::~Material(void)
{
	
}


}//namespace RenderData { 

}//namespace Squirrel {

