#pragma once

#include <common/common.h>
#include <Reflection/AtomicWrapper.h>
#include <Reflection/EnumWrapper.h>
#include <Reflection/Object.h>
#include <GUI/Container.h>
#include "macros.h"

namespace Squirrel {
namespace Editor { 

class ReflectObjectInspector;

struct ReflectFieldNode
{
	ReflectFieldNode(): member(NULL), label(NULL), readOnly(false)	{}
	const Reflection::Object::Field *	member;
	GUI::Element *						label;
	std::vector<GUI::Element *>			edits;
	bool								readOnly;
};

class ReflectFieldDelegate
{
public:
	virtual bool valueChanged(ReflectFieldNode& node, int index)	= 0;
	virtual bool fetchToUI(ReflectFieldNode& node, tuple2i& fieldSize, ReflectObjectInspector * inspector)					= 0;
};

class SQEDITOR_API ReflectObjectInspector: 
	public GUI::ActionDelegate
{
	Reflection::Object * mInspectingObject;

	GUI::Container * mTargetUIContainer;

	float mStartYPos;

	typedef std::list<ReflectFieldNode> MEMBER_NODES;

	MEMBER_NODES mMemberNodes;
	bool mStretchContainer;

	void cleanUp();
	void fill();

	tuple2i fillNode(ReflectFieldNode& node);

	void valueChanged(ReflectFieldNode& node, int value);

	ReflectFieldNode * findNodeForMember(Reflection::Object::Field * member);

	ReflectFieldDelegate * mReflectFieldDelegate;

public://methods

	ReflectObjectInspector();
	virtual ~ReflectObjectInspector();

	void setReflectFieldDelegete(ReflectFieldDelegate * reflFieldDel) { mReflectFieldDelegate = reflFieldDel; }

	inline Reflection::Object * getInspectingObject() { return mInspectingObject; }

	void setInspectingObject(Reflection::Object * obj);

	void init(GUI::Container * targetUIContainer, float startYPos);
	void update();

	virtual bool processAction(const std::string& action, GUI::Element * sender);

	static tuple2i MinAndMaxFieldWidth(Reflection::AtomicWrapper::EType type);

//hide it from all excepting ReflectFieldDelegate ancestors

	tuple2i fillAtomField(ReflectFieldNode& node, Reflection::AtomicWrapper * atom);
};

}//namespace Editor { 
}//namespace Squirrel {
