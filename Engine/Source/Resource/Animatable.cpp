#include "Animation.h"
#include "Animatable.h"
#include <Common/macros.h>

namespace Squirrel {

namespace Resource { 

Animatable::Animatable() 
{
}

Animatable::~Animatable()
{
	for(int i = 0; i < mTargetsMap.getSize(); ++i)
	{
		TargetInfo * trgInfo = mTargetsMap.get(i);
		DELETE_PTR(trgInfo);
	}
}


}//namespace Resource { 

}//namespace Squirrel {