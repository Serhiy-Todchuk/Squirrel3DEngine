#pragma once

#include "macros.h"

namespace Squirrel {
namespace Reflection {

class SQREFLECTION_API XMLCommon
{
public://methods

	static const char * ValuesNumAttrName()		{ return "ValuesNum"; }
	static const char * AtomValueNodeName()		{ return "Value"; }
	static const char * NameAttrName()			{ return "Name"; }
	static const char * ClassAttrName()			{ return "Class"; }
	static const char * ObjectElementName()		{ return "Object"; }
};

}//namespace Reflection {
}//namespace Squirrel