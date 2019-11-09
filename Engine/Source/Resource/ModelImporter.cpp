#include "ModelImporter.h"

namespace Squirrel {

namespace Resource { 

IModelImporter::IModelImporter(Render::IRender * render, Resource::TextureStorage * texStorage, Resource::MaterialLibrary * matLib):
  mRender(render), mTexturesStorage(texStorage), mMaterialLibrary(matLib) 
{}
IModelImporter::~IModelImporter() 
{}

}//namespace Resource { 

}//namespace Squirrel {