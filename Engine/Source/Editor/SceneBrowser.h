#pragma once

#include <common/common.h>
#include <Common/WindowDialogDelegate.h>
#include <GUI/Panel.h>
#include <GUI/List.h>
#include <GUI/Button.h>
#include <GUI/MenuContentSource.h>
#include <World/SceneObject.h>
#include <World/World.h>
#include <set>
#include "macros.h"

namespace Squirrel {
namespace Editor { 

class SQEDITOR_API SceneBrowserDelegate
{
public:
	SceneBrowserDelegate() {}
	virtual ~SceneBrowserDelegate() {}

	virtual void didSelectObject(World::SceneObject *object) = 0;
	virtual void didDoubleclickObject(World::SceneObject *object) = 0;
};

class SQEDITOR_API SceneBrowser: 
	public GUI::ListContentSource,
	public GUI::ActionDelegate,
	protected WindowDialogDelegate
{
	typedef std::set<World::SceneObject *> SCENE_OBJECTS_SET;

	SCENE_OBJECTS_SET mExpandedItems; 

	Math::vec3 mInstantiationPos; 

public://ctor/dtor
	SceneBrowser();
	~SceneBrowser();

public://methods

	void setInstantiationPos(Math::vec3 pos) { mInstantiationPos = pos; }

	//mResourceOpenDelegate setter
	void setSceneBrowserDelegate(SceneBrowserDelegate * delg) { mSceneBrowserDelegate = delg; }

	//ContentRoot setter
	void setContentRoot(World::World * cont) { mContentRoot = cont; checkoutContent(); }

	//main control
	void show();
	void close();
	void checkoutContent();

	void deleteSelected();

private://members

	//UI content
	GUI::Panel * mPanel;
	GUI::List * mList;

	GUI::Button * mWorldMenuButton;
	GUI::Button * mAddObjectMenuButton;

	GUI::MenuContentSource * mObjectMenu;
	GUI::MenuContentSource * mWorldMenu;
	GUI::MenuContentSource * mAddObjectMenu;
	GUI::MenuContentSource * mAddBehaviourMenu;

	//callback
	SceneBrowserDelegate * mSceneBrowserDelegate;

	//content src
	World::World *mContentRoot;

	std::vector<World::SceneObject *> mSceneList;

private://methods

	void			newWorld();
	void			openWorld();
	void			saveWorld();
	void			saveResources();

	void			duplicateSelected();
	void			deleteSelectedInternal();
	void			addBehaviour(const std::string& action, GUI::Element * sender);

	void			createShape(const std::string& action, GUI::Element * sender);
	void			createObject(const std::string& action, GUI::Element * sender);

	void			showWorldMenu(const std::string& action, GUI::Element * sender);
	void			showAddMenu(const std::string& action, GUI::Element * sender);

	bool			isExpanded(World::SceneObject * obj);
	void			expand(World::SceneObject * obj, bool expanded);

	//ListContentSource implementation
	size_t			getRowsNum();
	void			setupCell(GUI::Element * cell, int row);

	//ActionDelegate implementation
	virtual bool processAction(const std::string& action, GUI::Element * sender);

	//internal utils
	void appendListWithChildren(World::SceneObjectsContainer * objs);
	void rebuildList();
	void checkoutChildren(SCENE_OBJECTS_SET& dst, World::SceneObjectsContainer * src);
	void selectObject();
	
	void windowDialogEnd(int answer, void * contextInfo);
};

}//namespace Editor { 
}//namespace Squirrel {
