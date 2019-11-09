#pragma once

#include <Render/Image.h>
#include <Common/Data.h>
#include <string>
#include "macros.h"

namespace Squirrel {

namespace Resource { 

class SQRESOURCE_API ImageLoader
{

	bool mInitialized;

public:
	ImageLoader(void);
	~ImageLoader(void);

	bool mForceBGR2RGB;

	void init();
	RenderData::Image* loadImage(std::string fileName);
	RenderData::Image* loadImage(Data * data);
};


}//namespace Resource { 

}//namespace Squirrel {