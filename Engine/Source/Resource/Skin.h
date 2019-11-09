#pragma once

#include <Math/mat4.h>
#include <Common/tuple.h>
#include <Common/BufferArray.h>
#include "macros.h"

namespace Squirrel {

namespace Resource { 

//skin is a connection between skeleton and mesh
class SQRESOURCE_API Skin
{
public:

	//joint contains Vertex2Bones connection info
	//!joint is not a bone! bone is part of skeleton, joint is a part of skin.
	//joint.count is bones number which are infuencing vertex
	//joint.values[n].x is weight index
	//joint.values[n].y is bone index in skeleton
	//n is vertex index in mesh == joint index here
	typedef BufferArray<tuple2i> Joint;

	typedef BufferArray<Joint> JointsBuffer;

public:
	Skin(void);
	~Skin(void);

	void clampNumberOfBonesPerVertex(int bonesPerVertex = 4);

	BufferArray<float>			weights;
	JointsBuffer				joints;
	BufferArray<std::string>	boneNames;

private:

};


}//namespace Resource { 

}//namespace Squirrel {