#include "Behaviour.h"
#include <Reflection/AtomicWrapper.h>

namespace Squirrel {

namespace World { 

//
//static members
//

Behaviour::CLASSNAMES_SET& Behaviour::GetRegisteredBehaviourClasses()
{
	static CLASSNAMES_SET sRegisteredBehaviourClasses;
	return sRegisteredBehaviourClasses;
}

Behaviour * Behaviour::InstantiateBehaviour(const std::string& className)
{
	CLASSNAMES_SET::iterator it = GetRegisteredBehaviourClasses().find( className );
	if(it != GetRegisteredBehaviourClasses().end())
	{
		return static_cast<Behaviour *>(Reflection::Object::Instantiate( className ));
	}
	return NULL;
}

//
//ctor/dtor
//

Behaviour::Behaviour()
{
	mSceneObject	= 0;
	mHasStarted		= false;

	mEnabled = true;

	mClassNamesStack.push_back( std::string("World::Behaviour") );

	wrapAtomicField("Enabled", &mEnabled, 1);
}

Behaviour::~Behaviour()
{
}

}//namespace World { 

}//namespace Squirrel {
