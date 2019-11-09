#pragma once

#include "Element.h"
#include <memory>

namespace Squirrel {
namespace GUI { 

class SQGUI_API LayoutNode
{
public:

	typedef std::list<std::unique_ptr<LayoutNode> > LAYOUT_NODES_LIST;

	enum LayoutType
	{
		ltHorizontal,
		ltVertical
	};

public:
	LayoutNode();
	virtual ~LayoutNode();

private:

	LAYOUT_NODES_LIST mNodes;


};

class SQGUI_API Layout
{
public:
	Layout();
	virtual ~Layout();

	void update(tuple2i size);


};

}//namespace GUI { 
}//namespace Squirrel {