#pragma once

#include "AtomicWrapper.h"
#include <list>

namespace Squirrel {
namespace Reflection {

class EnumWrapper: public AtomicWrapperImpl<int32>
{
	struct ValueString
	{
		int32		value;
		std::string	str;
	};

	std::list<ValueString> mValueStringMap;

public://ctor
	EnumWrapper(int32 *values, int valuesNum = 1): AtomicWrapperImpl<int32>(values, valuesNum) {}
	virtual ~EnumWrapper() {}

	int32 findValueForString(const std::string& str) const
	{
		int defaultValue = -1;
		if(mValueStringMap.size() > 0)
		{
			defaultValue = mValueStringMap.begin()->value;
		}

		for(std::list<ValueString>::const_iterator it = mValueStringMap.cbegin();
			it != mValueStringMap.cend(); ++it)
			if(it->str == str)
				return it->value;

		return defaultValue;
	}

	std::string findStringForValue(int32 value) const
	{
		std::string defaultStr = "";
		if(mValueStringMap.size() > 0)
		{
			defaultStr = mValueStringMap.begin()->str;
		}

		for(std::list<ValueString>::const_iterator it = mValueStringMap.cbegin();
			it != mValueStringMap.cend(); ++it)
			if(it->value == value)
				return it->str;

		return defaultStr;
	}

	void addStringForValue(int32 value, const std::string& str)
	{
		ValueString valStr;
		valStr.value = value;
		valStr.str = str;
		mValueStringMap.push_back(valStr);
	}

	virtual std::string getStringRepresentation(int index = 0) const
	{
		ASSERT( index < mValuesNum );
		int32& value = static_cast<int32*>(mValues)[ index ];
		return findStringForValue(value);
	}

	virtual void parseString(const std::string& source, int index = 0)
	{
		ASSERT( index < mValuesNum );
		int32& value = static_cast<int32*>(mValues)[ index ];
		value = findValueForString(source);
	}

public://methods
};

}//namespace Reflection {
}//namespace Squirrel