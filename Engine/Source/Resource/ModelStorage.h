#pragma once

#include "Model.h"
#include "ResourceStorage.h"

namespace Squirrel {

namespace Resource { 

#ifdef _WIN32
SQRESOURCE_TEMPLATE template class SQRESOURCE_API ResourceStorage<Model>;
#endif
	
class SQRESOURCE_API ModelStorage: 
	public ResourceStorage<Model>
{
	static ModelStorage * sActiveLibrary;

public:
	ModelStorage();
	virtual ~ModelStorage();

	void setAsActive();
	static ModelStorage * Active();

protected:
	virtual bool save(Model* resource, Data * data, std::string& fileName);
	virtual Model* load(Data * data);
};


}//namespace Resource { 

}//namespace Squirrel {