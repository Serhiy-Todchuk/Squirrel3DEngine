// Switch.cpp: implementation of the Switch class.
//
//////////////////////////////////////////////////////////////////////

#include "Switch.h"
#include "Render.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Switch, GUISwitch);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


Switch::Switch()
{
	SQREFL_SET_CLASS(GUI::Switch);
	setSizeSep(40,16);
}

Switch::~Switch()
{

}

Element * Switch::onLU()
{
	return this;
}
Element * Switch::onRU()
{
	
	return this;
}

Element * Switch::onLD()
{
	//toggle state
	setState( getState() !=	Element::stateActive ? 
								Element::stateActive : 
								Element::stateStd );
	processAction(VALUE_CHANGED_ACTION, this);
	return this;
}

Element * Switch::onRD()
{

	return this;
}

void Switch::resetState()
{
	//empty
}

}//namespace GUI { 
}//namespace Squirrel {