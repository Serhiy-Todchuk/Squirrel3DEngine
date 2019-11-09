#pragma once

#include "ModelStorage.h"
#include "TextureStorage.h"
#include "ProgramStorage.h"
#include "SoundStorage.h"
#include <memory>

namespace Squirrel {
namespace Resource { 

class SQRESOURCE_API ResourceManager
{
	std::auto_ptr<MaterialLibrary>	mMaterialLibrary;
	std::auto_ptr<TextureStorage>	mTextureStorage;
	std::auto_ptr<ModelStorage>		mModelStorage;
	std::auto_ptr<ProgramStorage>	mProgramStorage;
	std::auto_ptr<SoundStorage>		mSoundStorage;

public:
	ResourceManager();
	virtual ~ResourceManager();

	void bind();
	void initContentSource(const std::string& section);

	inline MaterialLibrary *	getMaterialLibrary()	{ return mMaterialLibrary.get(); }
	inline TextureStorage *		getTextureStorage()		{ return mTextureStorage.get(); }
	inline ModelStorage *		getModelStorage()		{ return mModelStorage.get(); }
	inline ProgramStorage *		getProgramStorage()		{ return mProgramStorage.get(); }
	inline SoundStorage *		getSoundStorage()		{ return mSoundStorage.get(); }

	inline void	setMaterialLibrary	(MaterialLibrary *	ml)	{ mMaterialLibrary.reset(ml); }
	inline void	setTextureStorage	(TextureStorage *	ts)	{ mTextureStorage.reset(ts); }
	inline void	setModelStorage		(ModelStorage *		ms)	{ mModelStorage.reset(ms); }
	inline void	setProgramStorage	(ProgramStorage *	ps)	{ mProgramStorage.reset(ps); }
	inline void	setSoundStorage		(SoundStorage *		ss)	{ mSoundStorage.reset(ss); }
};


}//namespace Resource { 
}//namespace Squirrel {