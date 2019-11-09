#include "Serializable.h"

namespace Squirrel {
namespace Reflection {

Serializable::Serializable()
{
}

Serializable::~Serializable()
{
}

const std::string& Serializable::getClassName() const
{
	return mClassNamesStack.back();
}

bool Serializable::isKindOfClass(const std::string& className) const
{
	for(NAMES_LIST::const_iterator it = mClassNamesStack.begin(); it != mClassNamesStack.end(); ++it)
	{
		if((*it) == className) return true;
	}
	return false;
}

}//namespace Reflection {
}//namespace Squirrel