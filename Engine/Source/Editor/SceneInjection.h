#pragma once

#include <common/common.h>
#include <World/Behaviour.h>
#include "macros.h"

namespace Squirrel {
namespace Editor { 

class SQEDITOR_API SceneInjection: 
	public World::Behaviour
{
public://ctor/dtor
	SceneInjection();
	virtual ~SceneInjection();

	World::SceneObject * mSelectedObject;

public://methods
	virtual void awake()				{ };
	virtual void start()				{ };
	virtual void update()				{ };
	virtual void onBecomeVisible()		{ };
	virtual void onBecomeInvisible()	{ };
	virtual void debugRender()			{ };

	World::SceneObject * getSelectedObject() { return mSelectedObject; }
	void setSelectedObject(World::SceneObject * obj) { mSelectedObject = obj; }

};

}//namespace Editor { 
}//namespace Squirrel {
