#include "AnimationImporter.h"
#include <Common/Log.h>

namespace Squirrel {

using Resource::AnimationTrack;

namespace DAEImport { 

#define ANIMATION_TAG		"animation"
#define SAMPLER_TAG			"sampler"
#define CHANNEL_TAG			"channel"

#define INPUT_SEM			"INPUT"
#define OUTPUT_SEM			"OUTPUT"
#define INTERPOLATION_SEM	"INTERPOLATION"
#define IN_TANGENT_SEM		"IN_TANGENT"
#define OUT_TANGENT_SEM		"OUT_TANGENT"

#define INTERPOLATION_LINEAR	"LINEAR"
#define INTERPOLATION_BEZIER	"BEZIER"
#define INTERPOLATION_STEP		"STEP"
#define INTERPOLATION_CARDINAL	"CARDINAL"
#define INTERPOLATION_HERMITE	"HERMITE"
#define INTERPOLATION_BSPLINE	"BSPLINE"

AnimationImporter::AnimationImporter(void)
{
}

AnimationImporter::~AnimationImporter(void)
{
}

void AnimationImporter::importObject(pugi::xml_node node)
{
	if(std::string(node.name()) != ANIMATION_TAG) return;

	//recursively import sub-anims
	pugi::xml_node subAnimNode = node.child(ANIMATION_TAG);
	while(subAnimNode)
	{
		importObject(subAnimNode);
		subAnimNode = subAnimNode.next_sibling(ANIMATION_TAG);
	}

	//The <animation> element must contain <sampler> and <channel>
	if(!node.child(SAMPLER_TAG) || !node.child(CHANNEL_TAG))
		return;

	pugi::xml_node animNode = node;

	std::map<UniqueId, SourceNode *> dataArrays;

	//parse content

	//parse sources
	for(node = animNode.child(SOURCE_TAG.c_str()); node; node = node.next_sibling(SOURCE_TAG.c_str()))
	{
		SourceNode * sourceNode = readSourceNode(node);

		if(sourceNode)
		{
			dataArrays[sourceNode->id] = sourceNode;
		}
	}

	//parse samplers
	for(node = animNode.child(SAMPLER_TAG); node; node = node.next_sibling(SAMPLER_TAG))
	{
		//SAMPLER means ANIMATION TRACK in the engine

		UniqueId samplerId = node.attribute(ID_ATTR.c_str()).value();

		FloatArray * inputData		= NULL;
		FloatArray * outputData		= NULL;
		NameArray * interpData		= NULL;
		FloatArray * inTangentData	= NULL;
		FloatArray * outTangentData	= NULL;

		for(pugi::xml_node inputNode = node.child(INPUT_TAG.c_str()); inputNode; inputNode = inputNode.next_sibling(INPUT_TAG.c_str()))
		{
			String semantic = inputNode.attribute(SEMANTIC_ATTR.c_str()).value();

			UniqueId sourceIdLink = inputNode.attribute(SOURCE_ATTR.c_str()).value();
			UniqueId sourceId = sourceIdLink.substr(1);//skip first character as it is '#'

			ASSERT(dataArrays.find(sourceId) != dataArrays.end());

			SourceNode * sourceNode = dataArrays[sourceId];

			if(semantic == INPUT_SEM) {
				inputData = sourceNode->floatArray;
				ASSERT(inputData != NULL);
				if(sourceNode->name != "TIME")
				{
					Log::Instance().warning("AnimationImporter::importObject", (String("Import of other physical dimension then TIME not supported: ") + sourceNode->name).c_str());
				}
			} 
			else if(semantic == OUTPUT_SEM) {
				outputData = sourceNode->floatArray;
				ASSERT(outputData != NULL);
			}
			else if(semantic == INTERPOLATION_SEM) {
				interpData = sourceNode->namesArray;
				ASSERT(interpData != NULL);
			}
			else if(semantic == IN_TANGENT_SEM) {
				inTangentData = sourceNode->floatArray;
				ASSERT(inTangentData != NULL);
			}
			else if(semantic == OUT_TANGENT_SEM) {
				outTangentData = sourceNode->floatArray;
				ASSERT(outTangentData != NULL);
			}
		}

		//create anim track

		//create track
		int targetComponentsNum = outputData->getStride();
		int framesNum = inputData->getCount();
		AnimationTrack * track = new AnimationTrack(NULL, targetComponentsNum, framesNum);
		ASSERT(track);

		//store anim track
		std::map<UniqueId, AnimationTrack *>::iterator itAnimTrack = mAnimTracksMap.find(samplerId);
		if(itAnimTrack != mAnimTracksMap.end())
		{
			delete track;
			continue;
		}
		else
		{
			mAnimTracksMap[samplerId] = track;
		}

		//setup interp type
		//!!!don't support mixed types of interpolation
		String interpType = (interpData != nullptr && interpData->getSize() > 0) ? interpData->getElement(0) : INTERPOLATION_LINEAR;
		if(interpType == INTERPOLATION_LINEAR) {
			track->setInterpolationType(AnimationTrack::LINEAR);
		}
		else if(interpType == INTERPOLATION_BEZIER) {
			track->setInterpolationType(AnimationTrack::BEZIER);

			//support ony 
			if(inTangentData->getStride() != 2 || inTangentData->getCount() != outputData->getCount())
			{
				Log::Instance().warning("AnimationImporter::importObject", "Standard cubic bezier interpolation is only supported! Switching to linear interpolation.");
				track->setInterpolationType(AnimationTrack::LINEAR);
			}
		}
		else if(interpType == INTERPOLATION_STEP) {
			track->setInterpolationType(AnimationTrack::STEP);
		}
		else {
			//TODO: add support of CARDINAL, HERMITE and BSPLINE
			track->setInterpolationType(AnimationTrack::LINEAR);
		}

		//extract timeline
		for(uint i = 0; i < inputData->getSize(); ++i)
		{
			(*track->getTimeline())[i] = inputData->getElement(i);
		}

		//extract framesData
		int frameIndex = 0;
		int frameDataIndex = 0;
		for(uint i = 0; i < outputData->getSize(); ++i)
		{
			(*track->getFramesData())[frameIndex][frameDataIndex] = outputData->getElement(i);
			if(++frameDataIndex >= targetComponentsNum)
			{
				++frameIndex;
				frameDataIndex = 0;
			}
		}

		//extract tangents
		if(inTangentData != NULL)
		{
			track->getTangentsData()->resize( inTangentData->getSize() );
			for(uint i = 0; i < inTangentData->getSize(); ++i)
			{
				(*track->getTangentsData())[i].x = inTangentData->getElement(i);
				(*track->getTangentsData())[i].y = outTangentData->getElement(i);
			}
		}

		//fill up target indices
		Resource::AnimationTrack::INT_ARR * targetInds = track->getTargetIndices();
		for(int i = 0; i < targetComponentsNum; ++i)
		{
			(*targetInds)[i] = i;
		}
	}

	//parse channels
	for(node = animNode.child(CHANNEL_TAG); node; node = node.next_sibling(CHANNEL_TAG))
	{
		UniqueId samplerIdLink = node.attribute(SOURCE_ATTR.c_str()).value();
		UniqueId samplerId = samplerIdLink.substr(1);//skip first character as it is '#'

		//store anim track
		std::map<UniqueId, AnimationTrack *>::iterator itAnimTrack = mAnimTracksMap.find(samplerId);
		if(itAnimTrack == mAnimTracksMap.end()) continue;

		AnimChannel channel;
		//channel.id = samplerId;
		channel.animTrack = itAnimTrack->second;
		
		String targetString = node.attribute(TARGET_ATTR.c_str()).value();

		size_t pos = targetString.find_first_of('/');
		ASSERT(pos != String::npos);

		//extract id of target object
		channel.targetId = targetString.substr(0, pos);

		targetString = targetString.substr(pos + 1);

		//set default values
		channel.targetSubId = targetString;
		channel.targetMember = -1;

		const char memberAccessor = '.';

		//check if there is an access to members (through '.')
		pos = targetString.find_first_of(memberAccessor);
		if(pos != String::npos)
		{
			channel.targetSubId = targetString.substr(0, pos);

			String memberString = targetString.substr(pos + 1);

			if(memberString == "X") {
				channel.targetMember = 0;
			}
			else if(memberString == "Y") {
				channel.targetMember = 1;
			}
			else if(memberString == "Z") {
				channel.targetMember = 2;
			}
			else if(memberString == "ANGLE") {
				channel.targetMember = 3;
			}
		}

		const char arrayAccessOpening = '(';
		const char arrayAccessClosing = ')';

		//check if there is an array access (through '(' and ')')
		pos = targetString.find_first_of(arrayAccessOpening);
		if(pos != String::npos)
		{
			channel.targetSubId = targetString.substr(0, pos);

			targetString = targetString.substr(pos + 1);

			size_t closingPos = targetString.find_first_of(arrayAccessClosing);
			ASSERT(closingPos != String::npos);

			String indexString = targetString.substr(0, closingPos);

			channel.targetMember = atoi(indexString.c_str());

			//TODO: add support of 2D-arrays
		}

		mAnimChannelsMap[samplerId] = channel;
	}

	//cleanup
	for(std::map<UniqueId, SourceNode *>::iterator itDA = dataArrays.begin(); itDA != dataArrays.end(); ++itDA)
	{
		DELETE_PTR(itDA->second);
	}
}

const std::map<UniqueId, AnimationImporter::CHANNELS_LIST>& AnimationImporter::sortChannelsByTargetId()
{
	for(std::map<UniqueId, AnimChannel>::iterator itChannel = mAnimChannelsMap.begin(); itChannel != mAnimChannelsMap.end(); ++itChannel)
	{
		const UniqueId& targetId = itChannel->second.targetId;

		std::map<UniqueId, CHANNELS_LIST>::iterator itChannelsList = mChannelsByTargetId.find( targetId );

		if(itChannelsList == mChannelsByTargetId.end())
		{
			mChannelsByTargetId[targetId] = CHANNELS_LIST( 1, itChannel->second );
		}
		else
		{
			itChannelsList->second.push_back( itChannel->second );
		}
	}
	
	return mChannelsByTargetId;
}
/*
const AnimationImporter::AnimChannel * AnimationImporter::getAnimChannel(const UniqueId& fwAnimId) const
{
	std::map<UniqueId, AnimChannel>::const_iterator it = mAnimChannelsMap.find(fwAnimId);
	return it == mAnimChannelsMap.end() ? NULL : &it->second;
}
*/
const AnimationImporter::CHANNELS_LIST * AnimationImporter::getChannelsListForNode(const UniqueId& nodeId) const
{
	std::map<UniqueId, CHANNELS_LIST>::const_iterator it = mChannelsByTargetId.find(nodeId);
	return it == mChannelsByTargetId.end() ? NULL : &it->second;
}


}//namespace DAEImport { 
}//namespace Squirrel {