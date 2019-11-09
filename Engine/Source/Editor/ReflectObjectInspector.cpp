#include "ReflectObjectInspector.h"
#include <Render/IRender.h>
#include <GUI/Manager.h>
#include <GUI/Panel.h>
#include <GUI/Button.h>
#include <GUI/Label.h>
#include <GUI/Edit.h>
#include <GUI/Switch.h>
#include <GUI/IntField.h>
#include <GUI/FloatField.h>
#include <GUI/List.h>
#include <Reflection/XMLSerializer.h>
#include <Reflection/XMLDeserializer.h>
#include <limits>

namespace Squirrel {
namespace Editor {

using namespace Reflection;

#ifdef min
# undef min
#endif
#ifdef max
# undef max
#endif

tuple2i GetIntegerLimits(AtomicWrapper::EType integerType)
{
	tuple2i maxInt = tuple2i(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
	switch(integerType)
	{
	case AtomicWrapper::tInt8:
		return tuple2i(std::numeric_limits<int8>::min(), std::numeric_limits<int8>::max());
	case AtomicWrapper::tUInt8:
		return tuple2i(std::numeric_limits<uint8>::min(), std::numeric_limits<uint8>::max());
	case AtomicWrapper::tInt16:
		return tuple2i(std::numeric_limits<int16>::min(), std::numeric_limits<int16>::max());
	case AtomicWrapper::tUInt16:
		return tuple2i(std::numeric_limits<uint16>::min(), std::numeric_limits<uint16>::max());
	case AtomicWrapper::tInt32:
	case AtomicWrapper::tUInt32:
	case AtomicWrapper::tInt64:
	case AtomicWrapper::tUInt64:
	default:
		return maxInt;
	}
	return maxInt;
}

tuple2i ReflectObjectInspector::MinAndMaxFieldWidth(AtomicWrapper::EType type)
{
	tuple2i intFieldSizes(30, 90);
	tuple2i floatFieldSizes(40, 100);
	switch(type)
	{
	case AtomicWrapper::tBool:		return tuple2i(16, 16);
	case AtomicWrapper::tInt8:		return tuple2i(25, 60);
	case AtomicWrapper::tUInt8:		return tuple2i(25, 60);
	case AtomicWrapper::tInt16:		return tuple2i(30, 70);
	case AtomicWrapper::tUInt16:	return tuple2i(30, 70);
	case AtomicWrapper::tInt32:		return intFieldSizes;
	case AtomicWrapper::tUInt32:	return intFieldSizes;
	case AtomicWrapper::tInt64:		return intFieldSizes;
	case AtomicWrapper::tUInt64:	return intFieldSizes;
	case AtomicWrapper::tFloat32:	return floatFieldSizes;
	case AtomicWrapper::tFloat64:	return floatFieldSizes;
	case AtomicWrapper::tString:	return tuple2i(40, 1000);
	default:
		break;
	};
	return tuple2i(40, 120);
}

ReflectObjectInspector::ReflectObjectInspector()
{
	mInspectingObject = NULL;

	mTargetUIContainer = NULL;

	mStartYPos = 0;

	mStretchContainer = true;

	mReflectFieldDelegate = NULL;
}

ReflectObjectInspector::~ReflectObjectInspector()
{
	cleanUp();// :)
}

void ReflectObjectInspector::init(GUI::Container * targetUIContainer, float startYPos)
{
	mTargetUIContainer = targetUIContainer;
	mStartYPos = startYPos;
}

void ReflectObjectInspector::setInspectingObject(Reflection::Object * obj)
{
	if(mInspectingObject != obj)
	{
		cleanUp();
		mInspectingObject = obj;
		fill();
	}
}

void ReflectObjectInspector::valueChanged(ReflectFieldNode& node, int index)
{
	if(mReflectFieldDelegate != NULL)
	{
		if(mReflectFieldDelegate->valueChanged(node, index))
			return;
	}

	Serializable * object = node.member->object;

	if(object->isKindOfClass("EnumWrapper"))
	{
		EnumWrapper * enumWrapper = static_cast<EnumWrapper *>(object);
		
	}
	else
	{
		AtomicWrapper * atomicWrapper = static_cast<AtomicWrapper *>(object);

		switch(atomicWrapper->getType())
		{
		case AtomicWrapper::tBool:

			//update values
			{
				GUI::Switch * editField = static_cast<GUI::Switch *>(node.edits[index]);
				atomicWrapper->setValue<bool>( index, editField->getState() == GUI::Element::stateActive );
			}

			break;
		case AtomicWrapper::tInt8:
		case AtomicWrapper::tUInt8:
		case AtomicWrapper::tInt16:
		case AtomicWrapper::tUInt16:
		case AtomicWrapper::tInt32:
		case AtomicWrapper::tUInt32:
		case AtomicWrapper::tInt64:
		case AtomicWrapper::tUInt64:

			//update values
			{
				GUI::IntField * editField = static_cast<GUI::IntField *>(node.edits[index]);
				//special cases for char and uchar as their string representations are not numeric (but character)
				if(atomicWrapper->getType() == AtomicWrapper::tInt8)
					atomicWrapper->setValue<int8>(index, (int8)editField->getValue() );
				else if(atomicWrapper->getType() == AtomicWrapper::tUInt8)
					atomicWrapper->setValue<uint8>(index, (uint8)editField->getValue() );
				else
					atomicWrapper->parseString( editField->getText(), index );
			}
			break;
		case AtomicWrapper::tFloat32:
		case AtomicWrapper::tFloat64:

			//update values
			{
				GUI::FloatField * editField = static_cast<GUI::FloatField *>(node.edits[index]);
				atomicWrapper->parseString( editField->getText(), index );
			}
			break;
		case AtomicWrapper::tString:

			//update values
			{
				GUI::Edit * editField = static_cast<GUI::Edit *>(node.edits[index]);
				atomicWrapper->parseString( editField->getText(), index );
			}

			break;
		default:
			break;
		}
	}

}

tuple2i ReflectObjectInspector::fillAtomField(ReflectFieldNode& node, Reflection::AtomicWrapper * atom)
{
	int i = 0;

	bool create = node.edits.size() == 0;

	if(create)
		node.edits.resize(atom->getValuesNum(), NULL);

	if(node.readOnly)
	{
		if(create)
		{
			for(i = 0; i < atom->getValuesNum(); ++i)
			{
				GUI::Label * editField = mTargetUIContainer->add<GUI::Label>(node.member->name.c_str(), 0, 0, 10, 10);
				editField->setText("");
				node.edits[i] = editField;
			}
		}

		//update values
		for(i = 0; i < atom->getValuesNum(); ++i)
		{
			GUI::Label * editField = static_cast<GUI::Label *>(node.edits[i]);
			editField->setText( atom->getStringRepresentation(i) );
		}

		return MinAndMaxFieldWidth( atom->getType() );
	}

	switch(atom->getType())
	{
	case AtomicWrapper::tBool:

		//create if needed
		if(create)
		{
			for(i = 0; i < atom->getValuesNum(); ++i)
			{
				GUI::Switch * editField = mTargetUIContainer->add<GUI::Switch>(node.member->name.c_str(), 0, 0, 10, 10);
				editField->setActionDelegate(this);
				editField->setText("");
				node.edits[i] = editField;
			}
		}

		//update values
		for(i = 0; i < atom->getValuesNum(); ++i)
		{
			GUI::Switch * editField = static_cast<GUI::Switch *>(node.edits[i]);
			editField->setState( atom->getValue<bool>(i) ? GUI::Element::stateActive : GUI::Element::stateStd );
		}

		break;
	case AtomicWrapper::tInt8:
	case AtomicWrapper::tUInt8:
	case AtomicWrapper::tInt16:
	case AtomicWrapper::tUInt16:
	case AtomicWrapper::tInt32:
	case AtomicWrapper::tUInt32:
	case AtomicWrapper::tInt64:
	case AtomicWrapper::tUInt64:

		//create if needed
		if(create)
		{
			for(i = 0; i < atom->getValuesNum(); ++i)
			{
				GUI::IntField * editField = mTargetUIContainer->add<GUI::IntField>(node.member->name.c_str(), 0, 0, 10, 10);
				editField->setValueRange( GetIntegerLimits(atom->getType()) );
				editField->setText("0");
				editField->setActionDelegate(this);
				node.edits[i] = editField;
			}
		}

		//update values
		for(i = 0; i < atom->getValuesNum(); ++i)
		{
			GUI::IntField * editField = static_cast<GUI::IntField *>(node.edits[i]);
			//special cases for char and uchar as their string representations are not numeric (but character)
			if(atom->getType() == AtomicWrapper::tInt8)
				editField->setValue( atom->getValue<int8>(i) );
			else if(atom->getType() == AtomicWrapper::tUInt8)
				editField->setValue( atom->getValue<uint8>(i) );
			else
				editField->setValue( atoi( atom->getStringRepresentation(i).c_str() ) );
		}

		break;
	case AtomicWrapper::tFloat32:
	case AtomicWrapper::tFloat64:

		//create if needed
		if(create)
		{
			for(i = 0; i < atom->getValuesNum(); ++i)
			{
				GUI::FloatField * editField = mTargetUIContainer->add<GUI::FloatField>(node.member->name.c_str(), 0, 0, 10, 10);
				editField->setText("0.0");
				editField->setActionDelegate(this);
				node.edits[i] = editField;
			}
		}

		//update values
		for(i = 0; i < atom->getValuesNum(); ++i)
		{
			GUI::FloatField * editField = static_cast<GUI::FloatField *>(node.edits[i]);
			editField->setValue( (float)atof( atom->getStringRepresentation(i).c_str() ) );
		}

		break;
	case AtomicWrapper::tString:

		//create if needed
		if(create)
		{
			for(i = 0; i < atom->getValuesNum(); ++i)
			{
				GUI::Edit * editField = mTargetUIContainer->add<GUI::Edit>(node.member->name.c_str(), 0, 0, 10, 10);
				editField->setActionDelegate(this);
				node.edits[i] = editField;
			}
		}

		//update values
		for(i = 0; i < atom->getValuesNum(); ++i)
		{
			GUI::Edit * editField = static_cast<GUI::Edit *>(node.edits[i]);
			if(GUI::Element::GetFocus() != editField)
				editField->setText( atom->getStringRepresentation(i) );
		}

		break;
	default:
		break;
	}

	return MinAndMaxFieldWidth( atom->getType() );
}

tuple2i ReflectObjectInspector::fillNode(ReflectFieldNode& node)
{
	if(mReflectFieldDelegate != NULL)
	{
		tuple2i fieldSize(0, 0);
		if(mReflectFieldDelegate->fetchToUI(node, fieldSize, this))
			return fieldSize;
	}

	Serializable * object = node.member->object;
	bool create = node.edits.size() == 0;

	if(create)
	{
		node.readOnly = node.member->hasAttribute("Read-only");
	}

	/*
	if(object->isKindOfClass("CollectionWrapper"))
	{
		CollectionWrapper * collectionWrapper = static_cast<CollectionWrapper *>(object);

		GUI::List * editField = NULL;
		if(create)
		{
			node.edits.resize(1);

			editField = mTargetUIContainer->add<GUI::List>(node.member->name.c_str(), 0, 0, 10, 10);
			node.edits[0] = editField;
		}
		else
		{
			editField = static_cast<GUI::List *>(node.edits[0]);
		}

		//collectionWrapper->getSize();
	}
	else
	*/

	if(object->isKindOfClass("EnumWrapper"))
	{
		EnumWrapper * enumWrapper = static_cast<EnumWrapper *>(object);
		
	}
	else
	{
		AtomicWrapper * atomicWrapper = static_cast<AtomicWrapper *>(object);

		return fillAtomField(node, atomicWrapper);
	}

	return tuple2i(0, 0);
}

ReflectFieldNode * ReflectObjectInspector::findNodeForMember(Object::Field * member)
{
	for(MEMBER_NODES::iterator it = mMemberNodes.begin(); it != mMemberNodes.end(); ++it)
	{
		if(it->member == member)
			return &(*it);
	}

	return NULL;
}

void ReflectObjectInspector::fill()
{
	if(mInspectingObject == NULL) return;

	const float containerWidth = (float)mTargetUIContainer->getSize().x - mTargetUIContainer->getMargin().x;

	const float xSpace = 1;
	const float ySpace = 1;
	const float nextLineLeftMargin = 8;

	const float labelsColumnWidth = 0.3f * containerWidth;

	GUI::Font * normalFont = GUI::Render::Instance().getFont( GUI::Render::sizeNormal );
	const int controlHeight = normalFont->getCharHeight();

	float currY = mStartYPos;
	float currX = 0;

	const Object::FIELDS_MAP& members = mInspectingObject->getFields();
	Object::FIELDS_MAP::const_iterator memberIt;

	std::list<Object::Field *> sortedFields;
	std::list<Object::Field *>::iterator fieldIt;

	for(memberIt = members.begin(); memberIt != members.end(); ++memberIt)
	{
		sortedFields.push_back( const_cast<Object::Field *>(&memberIt->second) );
	}

	struct FieldsSorter {
		bool operator() (Object::Field * i,Object::Field * j) { return (i->index < j->index);}
	} fieldsSorter;

	sortedFields.sort(fieldsSorter);

	for(fieldIt = sortedFields.begin(); fieldIt != sortedFields.end(); ++fieldIt)
	{
		Object::Field * member = (*fieldIt);
		Serializable * object = member->object;

		if(!object->isKindOfClass("AtomicWrapper")) continue;

		ReflectFieldNode * memberNode = findNodeForMember(member);

		if(memberNode == NULL)
		{
			mMemberNodes.push_back(ReflectFieldNode());
			memberNode = &mMemberNodes.back();
			memberNode->member = member;
		}

		if(memberNode->label == NULL)
			memberNode->label = mTargetUIContainer->add<GUI::Label>(member->name.c_str(), 0, 0, 0, 0);

		memberNode->label->setPosSep((int)currX, (int)currY);
		memberNode->label->setSizeSep((int)labelsColumnWidth, (int)controlHeight);

		tuple2i minAndMaxWidthPerField = fillNode( *memberNode );

		size_t numberOfEdits = memberNode->edits.size();

		currX = labelsColumnWidth + xSpace;

		float widthForEdits = containerWidth - currX;
		float minEditWidth = minAndMaxWidthPerField.x + xSpace;
		float minEditsWidth = minEditWidth * numberOfEdits;

		//if there is not enough space for edits then move them to next line
		if(widthForEdits < minEditsWidth)
		{
			currY += controlHeight + ySpace;
			currX = nextLineLeftMargin;
			widthForEdits = containerWidth - currX;
		}

		float widthPerEdit = (widthForEdits / numberOfEdits) - xSpace;
		widthPerEdit = Math::minValue(widthPerEdit, (float)minAndMaxWidthPerField.y);

		for(int i = 0; i < numberOfEdits; ++i)
		{
			GUI::Element * editField = memberNode->edits[i];
			editField->setPosSep((int)currX, (int)currY);
			editField->setSizeSep((int)widthPerEdit, (int)controlHeight);

			currX += widthPerEdit + xSpace;
		}

		currY += controlHeight + ySpace;
		currX = 0;
	}

	tuple2i containerSize = mTargetUIContainer->getSize();
	containerSize.y = currY + mTargetUIContainer->getMargin().y + mTargetUIContainer->getMargin().w;
	mTargetUIContainer->setSize(containerSize);
}

void ReflectObjectInspector::cleanUp()
{
	uint i = 0;

	for(MEMBER_NODES::iterator it = mMemberNodes.begin(); it != mMemberNodes.end(); ++it)
	{
		mTargetUIContainer->del(it->label);
		DELETE_PTR(it->label);
		for(i = 0; i < it->edits.size(); ++i)
		{
			mTargetUIContainer->del(it->edits[i]);
			DELETE_PTR(it->edits[i]);
		}
		it->edits.clear();
	}
	mMemberNodes.clear();
}

void ReflectObjectInspector::update()
{
	if(mInspectingObject == NULL) return;

	//cleanUp();
	fill();
}

bool ReflectObjectInspector::processAction(const std::string& action, GUI::Element * sender)
{
	if(mInspectingObject == NULL) return false;

	bool changed = false;

	MEMBER_NODES::iterator memberNodeIt;

	for(memberNodeIt = mMemberNodes.begin(); memberNodeIt != mMemberNodes.end(); ++memberNodeIt)
	{
		for(uint i = 0; i < memberNodeIt->edits.size(); ++i)
		{
			if(sender == memberNodeIt->edits[i] && action == GUI::Element::VALUE_CHANGED_ACTION)
			{
				valueChanged((*memberNodeIt), i);
				if(memberNodeIt->member->changeFlag != NULL)
				{
					(*memberNodeIt->member->changeFlag) = true;
				}
				if(memberNodeIt->member->changeHandler.get() != NULL)
				{
					memberNodeIt->member->changeHandler.get()->execute();
				}
				changed = true;
				break;
			}
			if(changed)	break;
		}
	}

	return changed;
}

}//namespace Render { 
}//namespace Squirrel {

