#pragma once

#include "macros.h"
#include "../Common/Types.h"
#include "../Common/macros.h"
#include <string>
#include <list>
#include <map>
#include "Serializer.h"
#include "Deserializer.h"

namespace Squirrel {
namespace Reflection {

class SQREFLECTION_API Serializable
{
public://nested types

	typedef std::list<std::string>			NAMES_LIST;

public://ctor/dtor

	Serializable();
	virtual ~Serializable();

public://methods

	const std::string& getClassName() const;
	bool isKindOfClass(const std::string& className) const;

	virtual void serialize(Serializer * serializer)			= 0;
	virtual void deserialize(Deserializer * deserializer)	= 0;

protected://members

	NAMES_LIST		mClassNamesStack;
};

}//namespace Reflection {
}//namespace Squirrel