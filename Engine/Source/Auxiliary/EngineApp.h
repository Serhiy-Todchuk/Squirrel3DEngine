// EngineApp.h: interface for the EngineApp class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ResourcesApp.h"
#include <Engine/Engine.h>
#include <World/World.h>

namespace Squirrel {

namespace Auxiliary {
	
class EngineApp:
	public ResourcesApp
{
public:	
    /** Constructor */
	EngineApp(): mWorld(NULL), mEngine(NULL) {};
    /** Destructor */
	virtual ~EngineApp() {};
	
    virtual bool init()
    {
        bool success = ResourcesApp::init();

		if(Settings::Default() == NULL)
			getSettings()->setAsDefault();

        mEngine = new Engine::Engine;
        
        mDestructionPool->addObject(mEngine);
		
		initWorld();
        
        return success;
    }
	
	virtual void process()
	{
		mEngine->process(mWorld);
	}
	
protected:
	
	virtual void initWorld() = 0;
    
protected:
	
	Engine::Engine * mEngine;
	World::World * mWorld;
	
};

}//namespace Squirrel {
}//namespace Auxiliary {
