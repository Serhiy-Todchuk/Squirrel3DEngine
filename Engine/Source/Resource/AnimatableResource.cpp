#include "AnimatableResource.h"
#include "Animation.h"
#include "AnimationRunner.h"
#include <Common/macros.h>

namespace Squirrel {

namespace Resource { 

AnimatableResource::AnimatableResource() 
{

}

AnimatableResource::~AnimatableResource()
{
	for(ANIMS_MAP::iterator it = mAnimations.begin(); it != mAnimations.end(); ++it)
	{
		DELETE_PTR(it->second);
	}
}

Animation * AnimatableResource::getAnimation(const std::string& name) 
{
	ANIMS_MAP::iterator it = mAnimations.find(name);
	return it!=mAnimations.end() ? it->second : NULL;
}

Animation * AnimatableResource::createAnimation(const std::string& name)
{
	ANIMS_MAP::iterator it = mAnimations.find(name);
	if(it != mAnimations.end()) return it->second;

	Animation * anim = new Animation();
	mAnimations[name] = anim;
	return anim;
}


void AnimatableResource::fillAnimRunner(AnimationRunner * animRunner)
{
	if(mAnimations.size() == 0) return;

	ANIMS_MAP::iterator it;
	for(it = mAnimations.begin(); it != mAnimations.end(); ++it)
	{
		Animation * animRes = it->second;
		AnimationNode * animNode = new AnimationNode(animRes);
		animRunner->addAnimNode( it->first, animNode );
	}
}

bool AnimatableResource::loadAnims(Data * data)
{
	//remove existed
	for(ANIMS_MAP::iterator it = mAnimations.begin(); it != mAnimations.end(); ++it)
	{
		DELETE_PTR(it->second);
	}
	mAnimations.clear();

	//load anims
	int animsNum = data->readInt32();

	for(int i = 0; i < animsNum; ++i)
	{
		std::string animName;
		data->readString( animName, '\0' );
		Animation * anim = loadAnim( data );
		ASSERT( anim != NULL );
		anim->setName( animName );
		mAnimations[ animName ] = anim;
	}

	return true;
}

bool AnimatableResource::saveAnims(Data * data)
{
	data->putInt32( mAnimations.size() );

	for(ANIMS_MAP::iterator it = mAnimations.begin(); it != mAnimations.end(); ++it)
	{
		data->putString( it->first );
		ASSERT( saveAnim( data, it->second ) );
	}

	return true;
}

Animation * AnimatableResource::loadAnim(Data * data)
{
	int i = 0;

	Animation * anim = new Animation;

	long trackNum = data->readInt32();

	for(i = 0; i < trackNum; ++i)
	{
		_ID targetId = data->readInt32();
		
		//load frames size info
		int framesNum = data->readInt32();
		int frameSize = data->readInt32();

		//create track
		AnimationTrack * track = new AnimationTrack(NULL, frameSize, framesNum);

		//load erp type
		long erpType = data->readInt32();
		track->setInterpolationType( (AnimationTrack::InterpolationType)erpType );

		//load timeline (input)
		data->readBytes( &((*track->getTimeline())[0]), framesNum * sizeof(float) );

		//load framesData (output)
		for(int i = 0; i < framesNum; ++i)
		{
			data->readBytes( (*track->getFramesData())[i], frameSize * sizeof(float) );
		}

		//load target indices
		data->readBytes( &((*track->getTargetIndices())[0]), frameSize * sizeof(int) );

		//load tangents data
		long tangentsDataSize = data->readInt32();
		if(tangentsDataSize > 0)
		{
			track->getTangentsData()->resize( tangentsDataSize );
			data->readBytes( &((*track->getTangentsData())[0]), tangentsDataSize * sizeof(Math::vec2) );
		}

		//add track
		anim->addTrack( targetId, track );
	}

	int subAnimsNum = data->readInt32();

	for(i = 0; i < subAnimsNum; ++i)
	{
		Animation::SubAnim subAnim;
		data->readString( subAnim.mName, '\0' );
		subAnim.mRepeats = data->readInt32();
		subAnim.mTimeRange = data->readVar<Math::vec2>();
		anim->addSubAnim(subAnim);
	}

	anim->setFramesNum( data->readInt32() );

	return anim;
}

bool AnimatableResource::saveAnim(Data * data, Animation * anim)
{
	data->putInt32( anim->getTracks()->size() );

	for(Animation::TRACKS_MAP::iterator it = anim->getTracks()->begin(); it != anim->getTracks()->end(); ++it)
	{
		data->putInt32( it->mTargetId );
		
		//save track

		AnimationTrack * track = it->mTrack;

		//save frames size info
		size_t framesNum = track->getFramesData()->size();
		size_t frameSize = track->getTargetIndices()->size();
		data->putInt32( framesNum );
		data->putInt32( frameSize );

		//save erp type
		data->putInt32( track->getInterpolationType() );

		//save timeline (input)
		data->putData( &(*track->getTimeline())[0], framesNum * sizeof(float) );

		//save framesData (output)
		for(int i = 0; i < framesNum; ++i)
		{
			data->putData( (*track->getFramesData())[i], frameSize * sizeof(float) );
		}

		//save target indices
		data->putData( &(*track->getTargetIndices())[0], frameSize * sizeof(int) );

		//save tangents data
		size_t tangentsNum = track->getTangentsData()->size();
		data->putInt32( tangentsNum );
		if(tangentsNum > 0)
		{
			data->putData( &(*track->getTangentsData())[0], tangentsNum * sizeof(Math::vec2) );
		}
	}

	data->putInt32( anim->getSubAnims()->size() );

	Animation::SUBANIMS_MAP::iterator itSubAnims;
	for(	itSubAnims = anim->getSubAnims()->begin(); 
			itSubAnims != anim->getSubAnims()->end();
			++itSubAnims)
	{
		data->putString( itSubAnims->second.mName );
		data->putInt32( itSubAnims->second.mRepeats );
		data->putVar( itSubAnims->second.mTimeRange );
	}

	data->putInt32( anim->getFramesNum() );

	return true;
}


}//namespace Resource { 

}//namespace Squirrel {