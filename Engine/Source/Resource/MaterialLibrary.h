#pragma once

#include <Common/IDMap.h>
#include <Render/Material.h>
#include <Common/Data.h>
#include "macros.h"

namespace Squirrel {

namespace Resource { 

using namespace RenderData;

#ifdef _WIN32
SQRESOURCE_TEMPLATE template class SQRESOURCE_API IDMap<Material *>;
#endif
	
//TODO: create base class Library (analog of singleton template impl)
class SQRESOURCE_API MaterialLibrary:
	public IDMap<Material *>
{

static MaterialLibrary * sActiveLibrary;

public:
	MaterialLibrary();
	virtual ~MaterialLibrary();

	Material * getByName(std::string name);

	void setAsActive();
	static MaterialLibrary * Active();

	bool load(Data * data);
	bool save(Data * data);
};


}//namespace Resource { 

}//namespace Squirrel {