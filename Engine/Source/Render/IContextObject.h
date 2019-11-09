#pragma once

#include <Common/macros.h>
#include "macros.h"
#include <set>

#ifdef	_WIN32
//	disable warning on extern before template instantiation
#	pragma warning( disable: 4231 )
#endif

namespace Squirrel {

namespace Render {

//context depend object - object which could be generated/destroyed only when it's render device context is active
class SQRENDER_API IContextObject
{
public:

	typedef std::set<IContextObject *> OBJECTS_POOL;

public:
	IContextObject()			
	{ 
		mPool = NULL; 
	}

	virtual ~IContextObject()	
	{ 
		//derived class MUST call setPool(NULL) in it's destructor
		//ASSERT(mPool == NULL);
	}

	void setPool(OBJECTS_POOL * pool)
	{
		if(mPool != NULL)
		{
			destroy();
			mPool->erase(this);
		}
		if((mPool = pool) != NULL)
		{
			mPool->insert(this);
			generate();
		}
	}

	virtual void generate()	= 0;
	virtual void destroy()	= 0;

private:

	OBJECTS_POOL * mPool;

};

#ifdef _WIN32
SQRENDER_TEMPLATE template class SQRENDER_API std::set<IContextObject *>;
#endif
    
}//namespace Render { 

}//namespace Squirrel {

