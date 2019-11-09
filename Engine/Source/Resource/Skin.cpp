#include "Skin.h"

namespace Squirrel {

namespace Resource { 

Skin::Skin()
{
}

Skin::~Skin(void)
{
}

void Skin::clampNumberOfBonesPerVertex(int bonesPerVertex)
{
	int j = 0, i = 0;
	for(i = 0; i < joints.getCount(); ++i)
	{
		Skin::Joint& joint = joints.getData()[ i ];

		if(joint.getCount() > bonesPerVertex)
		{
			float weightSum = 0;
			for(j = 0; j < bonesPerVertex; ++j)
			{
				const tuple2i& boneInfluence = joint.getData()[ j ];
				float weight = weights.getData()[ boneInfluence.x ];
				weightSum += weight;
			}
			for(j = 0; j < bonesPerVertex; ++j)
			{
				const tuple2i& boneInfluence = joint.getData()[ j ];
				float& weight = weights.getData()[ boneInfluence.x ];
				weight /= weightSum;
			}
		}
	}
}

}//namespace Resource { 

}//namespace Squirrel {