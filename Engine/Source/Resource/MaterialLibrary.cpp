#include "MaterialLibrary.h"

namespace Squirrel {

namespace Resource { 

MaterialLibrary * MaterialLibrary::sActiveLibrary = NULL;

MaterialLibrary * MaterialLibrary::Active()
{
	return sActiveLibrary;
}

MaterialLibrary::MaterialLibrary()
{
	
}

MaterialLibrary::~MaterialLibrary()
{
	if(this == sActiveLibrary)
	{
		sActiveLibrary = NULL;
	}
}

void MaterialLibrary::setAsActive()
{
	sActiveLibrary = this;
}

Material * MaterialLibrary::getByName(std::string name)
{
	for(_ID id = 0; id < getSize(); ++id)
	{
		Material * mat = get(id);
		if(mat)
		{
			if(mat->getName() == name)
			{
				return mat;
			}
		}
	}
	return NULL;
}

bool MaterialLibrary::load(Data * data)
{
	int matCount = data->readInt32();
	
	for(int i = 0; i < matCount; ++i)
	{
		Material * mat = new Material();

		mat->setName		( data->readString() );
		mat->mLighting	=	( data->readString() );
		mat->mPhysics	=	( data->readString() );

		mat->mDiffuse	=	( data->readVar<Math::vec4>() );
		mat->mSpecular	=	( data->readVar<Math::vec4>() );
		mat->mEmission	=	( data->readVar<Math::vec4>() );
		mat->mAmbient	=	( data->readVar<Math::vec4>() );
		mat->mReflect	=	( data->readVar<Math::vec4>() );
		mat->mShininess	=	( data->readFloat() );
		mat->mOpacity	=	( data->readFloat() );

		_ID id = add( mat );
		mat->setID( id );
	}
	
	return true;
}

bool MaterialLibrary::save(Data * data)
{
	data->putInt32( getCount() );
	
	for(int i = 0; i < getSize(); ++i)
	{
		Material * mat = get(i);
		if(mat == NULL) continue;

		data->putString( mat->getName() );
		data->putString( mat->mLighting );
		data->putString( mat->mPhysics );

		data->putVar( mat->mDiffuse );
		data->putVar( mat->mSpecular );
		data->putVar( mat->mEmission );
		data->putVar( mat->mAmbient );
		data->putVar( mat->mReflect );
		data->putFloat( mat->mShininess );
		data->putFloat( mat->mOpacity );
	}

	return true;
}


}//namespace Resource { 

}//namespace Squirrel {