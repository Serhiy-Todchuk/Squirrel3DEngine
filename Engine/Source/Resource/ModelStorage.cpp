#include "ModelStorage.h"
#include <Common/Data.h>
#include <set>

namespace Squirrel {

namespace Resource { 

ModelStorage * ModelStorage::sActiveLibrary = NULL;

ModelStorage * ModelStorage::Active()
{
	return sActiveLibrary;
}

ModelStorage::ModelStorage()
{
	
}

ModelStorage::~ModelStorage()
{
	if(this == sActiveLibrary)
	{
		sActiveLibrary = NULL;
	}
}

#define _CURENT_VERSION					101

#define _COTAINER_KEY					0x2048

#define _COTAINER_MODEL_TAG				0x0011
#define _COTAINER_MATERIAL_PALETTE_TAG	0x0051
#define _COTAINER_SKELETON_TAG			0x0014

bool ModelStorage::save(Model* resource, Data * data, std::string& fileName)
{
	const int32 capacityIncr = 1024 * 16;//16 kBytes for increment
	data->setCapacityIncrement( capacityIncr );

	data->putInt32( _COTAINER_KEY );

	data->putInt32( _CURENT_VERSION );

	//save model
	return resource->save( data );
}

Model* ModelStorage::load(Data * data)
{
	Model * model = new Model;
	ASSERT( model != NULL );

	ASSERT( data != NULL );
	ASSERT( data->getLength() > 0 );

	int fileKey = data->readInt32();
	ASSERT( fileKey == _COTAINER_KEY );

	int version = data->readInt32();

	data->setVersion(version);

	model->load(data);

	DELETE_PTR(data);

	return model;
}

void ModelStorage::setAsActive()
{
	sActiveLibrary = this;
}


}//namespace Resource { 

}//namespace Squirrel {