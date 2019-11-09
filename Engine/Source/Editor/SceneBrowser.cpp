#include "SceneBrowser.h"
#include <Render/IRender.h>
#include <GUI/Manager.h>
#include <GUI/MenuItem.h>
#include <GUI/Label.h>
#include <GUI/Foldout.h>
#include <GUI/Window.h>
#include <GUI/MenuContentSource.h>
#include <World/Body.h>
#include <World/ParticleSystem.h>
#include <World/Light.h>
#include <World/SoundSource.h>
#include <Reflection/BinSerializer.h>
#include <Reflection/BinDeserializer.h>

namespace Squirrel {
namespace Editor {

using namespace Reflection;

SceneBrowser::SceneBrowser():
	mInstantiationPos(0,0,0), mSceneBrowserDelegate(NULL)
{
	mPanel = new GUI::Window();
	mPanel->setName("scenePanel");
	mPanel->setText("Scene Browser");
	mPanel->setPosSep(5, 375);
	mPanel->setSizeSep(244, 216);
	GUI::Manager::Instance().getMainPanel().addPanel( mPanel );

	mWorldMenuButton		= mPanel->add<GUI::Button>("World", 0, 1, 60, 16);
	mWorldMenuButton->addAction( GUI::Button::LEFT_CLICK_ACTION, this, &SceneBrowser::showWorldMenu );
	mAddObjectMenuButton	= mPanel->add<GUI::Button>("Create", 63, 1, 60, 16);
	mAddObjectMenuButton->addAction( GUI::Button::LEFT_CLICK_ACTION, this, &SceneBrowser::showAddMenu );

	mList = mPanel->add<GUI::List>("sceneBrowser", 0, 18, 240, 180);
	mList->setContentSource( this );
	mList->setActionDelegate( this );

	mList->setVerticalSizing(	GUI::Element::bindFlexible );
	mList->setHorizontalSizing(	GUI::Element::bindFlexible );
	mList->setBindingBottom(	GUI::Element::bindStrictly );
	mList->setBindingLeft(		GUI::Element::bindStrictly );
	mList->setBindingTop(		GUI::Element::bindStrictly );
	mList->setBindingRight(		GUI::Element::bindStrictly );

	//init cell creator
	ObjectCreator * cellCreator = ObjectCreator::GetCreator("GUI::Foldout");
	if(cellCreator == NULL)
	{
		cellCreator = ObjectCreatorFactory::Create<GUI::Foldout>("GUI::Foldout");
	}
	mList->setCellCreator(cellCreator);

	//hide
	mPanel->setVisible(false);

	mObjectMenu = new GUI::MenuContentSource();
	mObjectMenu->addItem("Duplicate").mHandler = new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::duplicateSelected);
	mObjectMenu->addItem("Delete").mHandler = new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::deleteSelectedInternal);
	GUI::MenuItemDesc& behavioursMenuItem = mObjectMenu->addItem("Add behaviour");

	mWorldMenu = new GUI::MenuContentSource();
	mWorldMenu->addItem("New").mHandler = new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::newWorld);
	mWorldMenu->addItem("Open").mHandler = new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::openWorld);
	mWorldMenu->addItem("Save").mHandler = new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::saveWorld);
	mWorldMenu->addItem("Save Resources").mHandler = new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::saveResources);
	mWorldMenu->addItem("Save As...");

	mAddObjectMenu = new GUI::MenuContentSource();
	mAddObjectMenu->addItem("Sphere"	).mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createShape);
	mAddObjectMenu->addItem("Cube"		).mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createShape);
	mAddObjectMenu->addItem("Cylinder"	).mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createShape);
	mAddObjectMenu->addItem("Torus"		).mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createShape);
	mAddObjectMenu->addItem("SemiSphere").mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createShape);
	mAddObjectMenu->addItem("World::SceneObject"	).mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createObject);
	mAddObjectMenu->addItem("World::Body"			).mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createObject);
	mAddObjectMenu->addItem("World::ParticleSystem"	).mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createObject);
	mAddObjectMenu->addItem("World::Light"			).mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createObject);
	mAddObjectMenu->addItem("World::SoundSource"	).mHandler	= new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::createObject);

	mAddBehaviourMenu = new GUI::MenuContentSource();
	for(World::Behaviour::CLASSNAMES_SET::iterator it = World::Behaviour::GetRegisteredBehaviourClasses().begin();
		it != World::Behaviour::GetRegisteredBehaviourClasses().end(); ++it)
	{
		mAddBehaviourMenu->addItem( (*it) ).mHandler = new GUI::ActionFunctor<SceneBrowser>(this, &SceneBrowser::addBehaviour);
	}

	behavioursMenuItem.mSubmenu = mAddBehaviourMenu;
}

SceneBrowser::~SceneBrowser()
{
	GUI::Manager::Instance().getMainPanel().delPanel( mPanel );
	DELETE_PTR(mPanel);

	DELETE_PTR(mObjectMenu);
	DELETE_PTR(mWorldMenu);
	DELETE_PTR(mAddObjectMenu);
}

void SceneBrowser::addBehaviour(const std::string& action, GUI::Element * sender)
{
	int selectedIndex = mList->getSelected();
	if(selectedIndex >= 0)
	{
		World::SceneObject * selectedObj = mSceneList[selectedIndex];

		World::Behaviour * behaviour = World::Behaviour::InstantiateBehaviour(sender->getText());

		if(behaviour)
		{
			selectedObj->addBehaviour(behaviour);
		}
	}
}
	
const char * newWorldDialogTitle = "New world!";
const char * deleteObjectDialogTitle = "Deleting scene object!";

void SceneBrowser::windowDialogEnd(int answer, void * contextInfo)
{
	if(answer == 0)
	{
	   if(contextInfo == newWorldDialogTitle)
	   {
		   if(mSceneBrowserDelegate != NULL)
			   mSceneBrowserDelegate->didSelectObject(NULL);
		
		   mContentRoot->reset();
		   rebuildList();
	   }
		
		if(contextInfo == deleteObjectDialogTitle)
		{
			int selectedIndex = mList->getSelected();
			if(selectedIndex >= 0)
			{
				World::SceneObject * selectedObj = mSceneList[selectedIndex];
				
				if(mSceneBrowserDelegate != NULL)
					mSceneBrowserDelegate->didSelectObject(NULL);
				
				World::SceneObjectsContainer * container = (selectedObj->getParent() != NULL) ? 
				                                           ((World::SceneObjectsContainer *)selectedObj->getParent()) : 
				                                           ((World::SceneObjectsContainer *)mContentRoot);
				container->delSceneObject( container->findSceneObjectIt(selectedObj) );
				
				rebuildList();
			}
		}
	}
}
	
void SceneBrowser::newWorld()
{
	Render::IRender::GetActive()->getWindow()->showDialog(newWorldDialogTitle, "Are you sure you want to clear world?", true, this);
}

void SceneBrowser::openWorld()
{
	Data * worldData = new Data("world.xml");
	mContentRoot->load(worldData);

	checkoutContent();
}

void SceneBrowser::saveWorld()
{
	Data * worldData = new Data(NULL, 0);
	worldData->setCapacityIncrement(4096);

	mContentRoot->save(worldData);
	worldData->writeToFile("world.xml");
}

void SceneBrowser::saveResources()
{
	Resource::TextureStorage::Active()->saveChangedResources();
	Resource::ModelStorage::Active()->saveChangedResources();
}

void SceneBrowser::duplicateSelected()
{
	int selectedIndex = mList->getSelected();
	if(selectedIndex >= 0)
	{
		World::SceneObject * selectedObj = mSceneList[selectedIndex];

		//save object
		Reflection::BinSerializer serializer;

		//TODO: create save method in Reflection::Object
		serializer.beginWriteObject( selectedObj->getClassName(), selectedObj->getName() );
		selectedObj->serialize(&serializer);
		serializer.endWriteObject();

		Reflection::DATA_PTR xmlData = serializer.getData();
		
		//move data memory ownership to Data obj
		xmlData->memoryOwner = false;
		Data * data = new Data(xmlData->data, xmlData->length);

		//load object
		Reflection::BinDeserializer deserializer;
		deserializer.loadFrom(data->getData(), data->getLength());

		World::SceneObject * clone = static_cast<World::SceneObject *>(Reflection::Object::Instantiate(&deserializer));

		mContentRoot->addSceneObject( clone );

		rebuildList();
	}
}

void SceneBrowser::deleteSelectedInternal()
{
	Render::IRender::GetActive()->getWindow()->showDialog(deleteObjectDialogTitle, "Are you sure you want to delete selected scene object?", true, this);
}

void SceneBrowser::deleteSelected()
{
	if(GUI::Element::GetFocus() && GUI::Element::GetFocus() != mList && GUI::Element::GetFocus()->getMaster() != mList)
		return;

	deleteSelectedInternal();
}

using RenderData::VertexBuffer;

void SceneBrowser::createShape(const std::string& action, GUI::Element * sender)
{
	float diameter = mContentRoot->getUnitsInMeter();
	float radius = diameter * 0.5f;

	Resource::MeshBuilder * builder = NULL;

	if(sender->getName() == "Sphere") {
		builder = new Resource::SphereBuilder(radius, 50, 50, VT_PN);
	} else if(sender->getName() == "Cube") {
		builder = new Resource::BoxBuilder(Math::vec3(-radius, -radius, -radius), Math::vec3(radius, radius, radius), VT_PN);
	} else if(sender->getName() == "Cylinder") {
		builder = new Resource::CylinderBuilder(diameter, radius, radius, 50, 50, VT_PN);
	} else if(sender->getName() == "Torus") {
		builder = new Resource::TorusBuilder(radius, radius * 0.4f, 50, 50, VT_PN);
	} else if(sender->getName() == "SemiSphere") {
		builder = new Resource::HalfSphereBuilder(radius, 50, 50, VT_PN);
	}

	if(builder == NULL)
		return;

	Resource::Mesh * mesh = builder->buildMesh();
	World::Body * body = new World::Body();
	body->initWithMesh(mesh);
	body->setName( sender->getName() );
	body->setLocalPosition(mInstantiationPos);

	mContentRoot->addSceneObject(body);

	rebuildList();
}

void SceneBrowser::createObject(const std::string& action, GUI::Element * sender)
{
	const std::string& className = sender->getText();
	World::SceneObject * object = static_cast<World::SceneObject *>( Reflection::Object::Instantiate( className ) );

	if(object == NULL)
		return;

	object->setName( sender->getName() );
	object->setLocalPosition(mInstantiationPos);

	mContentRoot->addSceneObject(object);

	rebuildList();
}

void SceneBrowser::show()
{
	rebuildList();
	//mList->update();
	mPanel->setVisible(true);
}

void SceneBrowser::close()
{
	mPanel->setVisible(false);
}

void SceneBrowser::checkoutChildren(SCENE_OBJECTS_SET& dst, World::SceneObjectsContainer * src)
{
	World::SceneObject::SCENE_OBJECTS_LIST::const_iterator it = src->getSceneObjects().begin();
	for(; it != src->getSceneObjects().end(); ++it)
	{
		World::SceneObject * obj = (*it); 
		if(isExpanded(*it))
		{
			dst.insert(obj);
			checkoutChildren(dst, obj);
		}
	}
}

void SceneBrowser::checkoutContent()
{
	SCENE_OBJECTS_SET newExpandedItems;
	checkoutChildren(newExpandedItems, mContentRoot);
	mExpandedItems.swap(newExpandedItems);

	rebuildList();
}

void SceneBrowser::appendListWithChildren(World::SceneObjectsContainer * src)
{
	World::SceneObject::SCENE_OBJECTS_LIST::const_iterator it = src->getSceneObjects().begin();
	for(; it != src->getSceneObjects().end(); ++it)
	{
		mSceneList.push_back(*it);
		if(isExpanded(*it))
		{
			appendListWithChildren(*it);
		}
	}
}

bool SceneBrowser::isExpanded(World::SceneObject * obj)
{
	return mExpandedItems.find(obj) != mExpandedItems.end();
}

void SceneBrowser::expand(World::SceneObject * obj, bool expanded)
{
	if(expanded)
	{
		mExpandedItems.insert(obj);
	}
	else
	{
		mExpandedItems.erase(obj);
	}
}

void SceneBrowser::rebuildList()
{
	mSceneList.clear();
	appendListWithChildren( mContentRoot );
}

size_t SceneBrowser::getRowsNum()
{
	return mSceneList.size();
}

void SceneBrowser::setupCell(GUI::Element * cell, int row)
{
	World::SceneObject * obj = mSceneList[row];

	bool expanded = isExpanded(obj);

	int depth = 0;
	World::SceneObject * parent = obj;
	while((parent = parent->getParent()))
	{
		++depth;
	}

	GUI::Foldout * foldout = static_cast<GUI::Foldout *>(cell);
	foldout->setIntendance(depth);
	foldout->setShowFold(obj->getSceneObjects().size() > 0);
	cell->setState(expanded ? GUI::Element::stateActive : GUI::Element::stateStd);
	cell->setText((char *)obj->getName().c_str());
}

void SceneBrowser::selectObject()
{
	int selectedIndex = mList->getSelected();
	if(mSceneBrowserDelegate != NULL)
	{
		if(selectedIndex >= 0)
		{
			mSceneBrowserDelegate->didSelectObject(mSceneList[selectedIndex]);
		}
		else
		{
			mSceneBrowserDelegate->didSelectObject(NULL);
		}
	}
}

void SceneBrowser::showWorldMenu(const std::string& action, GUI::Element * sender)
{
	GUI::Manager::Instance().showMenu(mWorldMenu);
}

void SceneBrowser::showAddMenu(const std::string& action, GUI::Element * sender)
{
	GUI::Manager::Instance().showMenu(mAddObjectMenu);
}

bool SceneBrowser::processAction(const std::string& action, GUI::Element * sender)
{
	if(GUI::Manager::Instance().getState() != GUI::Manager::stateProcessInput) return false;

	if(sender == NULL) return false;

	if(sender == mList)
	{
		if(action == GUI::List::RIGHTCLICKED_ACTION)
		{
			GUI::Manager::Instance().showMenu(mObjectMenu);

			return true;
		}
		if(action == GUI::List::DOUBLECLICKED_ACTION)
		{
			int selectedIndex = mList->getSelected();
			if(selectedIndex >= 0 && mSceneBrowserDelegate != NULL)
			{
				mSceneBrowserDelegate->didDoubleclickObject(mSceneList[selectedIndex]);
			}
			if(selectedIndex < 0) return false;

			return true;
		}
		else if(action == GUI::List::SELECTION_CHANGED_ACTION)
		{
			selectObject();
			return true;
		}
	}
	else if(sender->getMaster() == mList && action == GUI::Element::STATE_CHANGED_ACTION)
	{
		int expandCellIndex = sender->getId();
		bool expanded = sender->getState() == GUI::Element::stateActive;
		expand(mSceneList[expandCellIndex], expanded);
		rebuildList();
		return true;
	}
	return false;
}

}//namespace Render { 
}//namespace Squirrel {

