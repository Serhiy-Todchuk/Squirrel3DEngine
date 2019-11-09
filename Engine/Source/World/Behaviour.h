#pragma once

#include <Reflection/AtomicWrapper.h>
#include <Reflection/Object.h>
#include <set>
#include "macros.h"

namespace Squirrel {

namespace World { 

class SceneObject;

class SQWORLD_API Behaviour : 
	public Reflection::Object
{
public:
	friend class SceneObject;

	typedef std::set<std::string> CLASSNAMES_SET;

public:
	Behaviour();
	virtual ~Behaviour();

	virtual void awake()				{ };
	virtual void start()				{ };
	virtual void update()				{ };
	virtual void onBecomeVisible()		{ };
	virtual void onBecomeInvisible()	{ };
	virtual void renderDebugInfo()		{ };

	inline SceneObject * getSceneObject()	{ return mSceneObject; }

	bool isEnabled()				{ return mEnabled; }
	void setEnabled(bool enabled)	{ mEnabled = enabled; }

	template <class TBehaviour>
	static void RegisterBehaviourClass(const std::string& className)
	{
		GetRegisteredBehaviourClasses().insert(className);
		Reflection::ObjectCreatorFactory::Create<TBehaviour>(className);
	}
	static CLASSNAMES_SET& GetRegisteredBehaviourClasses();
	static Behaviour * InstantiateBehaviour(const std::string& className);

protected:

	SceneObject *	mSceneObject;

private:

	bool mHasStarted;
	bool mEnabled;
};

template <class TClass>
struct BehaviourRegistratorHelper
{
	BehaviourRegistratorHelper(const char * className)
	{
		Behaviour::RegisterBehaviourClass<TClass>(className);
	}
};

#define SQ_REGISTER_BEHAVIOUR(TClass)							Squirrel::World::BehaviourRegistratorHelper<TClass> TOKENPASTE2(_BehaviourRegistratorHelper, __COUNTER__)( #TClass );
#define SQ_REGISTER_BEHAVIOUR_SEED(TClass, Seed)				Squirrel::World::BehaviourRegistratorHelper<TClass> TOKENPASTE2(_BehaviourRegistratorHelper, Seed)( #TClass );

}//namespace World { 

}//namespace Squirrel {

