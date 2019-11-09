#pragma once

#include <Resource/Model.h>
#include "SceneObject.h"

namespace Squirrel {
namespace World {

class Skeleton;

class SQWORLD_API Body: 
	public SceneObject
{

public:
	Body(void);
	virtual ~Body(void);

	virtual bool findAllIntersections(Ray ray, RAYCASTHITS_LIST& outList);

	virtual void update(float dtime);
	virtual void renderDebugInfo(Render::IRender * render, Render::Camera * camera);
	virtual void render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info);

	//accessors

	Model *				getModel()		{ return mModel; }
	Model::Node *		getMesh()		{ return mMesh; }
	Skeleton *			getSkeleton()	{ return mSkeleton; }

	void initWithModel(Model * sourceModel);//
	void initWithMesh(Resource::Mesh * mesh);

	virtual void deserialize(Reflection::Deserializer * deserializer);

	virtual void saveSubAnims();

private:

	typedef std::list<Body *> BODIES_LIST;

	void invalidateChildren();
	void initMembers();
	void addChildren(Model::Node::NODE_LIST * sourceMeshNodes, Body * modelRoot, BODIES_LIST& skeletons);

	void setupSubordinate(Body * subordinate, BODIES_LIST& bodiesWithSkeletons);
	void setupMasterAnimations(const BODIES_LIST& bodiesWithSkeletons);

	void onModelNameChanged();

protected:

	virtual void calcAABB();

protected:

	//content members

	bool			  mMeshOwner;

	Model			* mModel;
	Model::Node		* mMesh;
	Skeleton		* mSkeleton;

	std::string		  mModelName;
	_ID				  mMeshId;
	bool			  mModelRoot;
};

}//namespace World { 
}//namespace Squirrel {

