#include "IRender.h"

namespace Squirrel {
namespace Render { 

UniformString IRender::sColorUniformName			("uColor");
UniformString IRender::sAlphaTestUniformName		("uAlphaTest");
UniformString IRender::sModelviewMatrixUniformName	("uModelViewMatrix");
UniformString IRender::sMVPMatrixUniformName		("uMVPMatrix");
UniformString IRender::sNormalMatrixUniformName		("uNormalMatrix");

IRender * IRender::sActiveRender = NULL;
tuple2i IRender::sScreenSize = tuple2i(0, 0);

IRender::IRender():
	mViewport(0, 0, 0, 0), mWindow(NULL)
{
}

IRender::~IRender() 
{
	//go through the copy of pool so objects will be able to unregister itself from actual pool
	IContextObject::OBJECTS_POOL pool(mContextObjects);

	for(IContextObject::OBJECTS_POOL::iterator it = pool.begin(); it != pool.end(); ++it)
	{
		(*it)->setPool(NULL);
	}
}

}//namespace Render {
} //namespace Squirrel {
