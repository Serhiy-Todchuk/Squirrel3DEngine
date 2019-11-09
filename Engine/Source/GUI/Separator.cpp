#include "Separator.h"
#include "Render.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Separator, GUISeparator);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Separator::Separator()
{
	SQREFL_SET_CLASS(GUI::Separator);
}

Separator::~Separator()
{
}

void Separator::draw()
{
	if(getDrawStyle() > 0)
	{
		tuple2i p = getGlobalPos();
		Render::Instance().drawRect(p, getSize(), Math::vec4(0.4f, 0.5f, 0.4f, 1));
	}
}

}//namespace GUI { 
}//namespace Squirrel {