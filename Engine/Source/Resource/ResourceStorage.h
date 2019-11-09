#pragma once

#include <Common/IDMap.h>
#include <FileSystem/FileStorageFactory.h>
#include <Common/Data.h>
#include <Common/Log.h>
#include <Common/Types.h>
#include <Common/Macros.h>
#include <string>
#include "macros.h"

namespace Squirrel {
namespace Resource {

#define _INVALID_ID	-1
	
#define RESOURCES_SETTINGS_SECTION		"Resources"

using namespace FileSystem;

class SQRESOURCE_API StoredObject
{
	std::string		mName;
	_ID				mID;
	uint			mUse;//reference counting
	bool			mChanged;
	time_t			mTimestamp;

public:
	StoredObject(): mUse(0), mID(_INVALID_ID), mChanged(false), mTimestamp(0) {}
	virtual ~StoredObject() {}

	const std::string&	getName	()	const		{return mName;}
	_ID		getID()	const		{ return mID;}
	uint	getUse() const		{ return mUse; }
	void	setName	(const std::string& name)	{mName	= name;}
	void	setUse(uint use)	{ mUse	= use; }
	void	setID(_ID id)		{ mID	= id;}

	uint	incrUse()			{ return ++mUse; }
	uint	decrUse()			{ return --mUse; }

	time_t	getTimestamp()		{ return mTimestamp; }
	void	setTimestamp(time_t t){ mTimestamp = t; }

	bool	isChanged()			{ return mChanged; }
	void	setChanged(bool flag = true)
	{
		mChanged = flag;
	}

private:
};

template<class _TResource>
class /*SQRESOURCE_API*/ ResourceStorage:
	protected IDMap<_TResource*>
{
protected:
	std::auto_ptr<FileStorage> mContentSource;
	std::string mExtension;
	bool mDirty;
	bool mAllowOverwriting;
	bool mMapFiles;

protected://abstract class - hide constructor
	ResourceStorage()
	{
		mExtension = "";
		mDirty = false;
		mAllowOverwriting = true;
		mMapFiles = true;
	}
public:
	virtual ~ResourceStorage()
	{
		for(_ID i = 0; i<IDMap<_TResource*>::getSize(); ++i)
		{
			_TResource* resource = IDMap<_TResource*>::get(i);
			DELETE_PTR(resource);
		}		
	}
	void initContentSource(std::string path)
	{
		FileStorage * contentSource = FileStorageFactory::GetFileStorageForPath( path );
		if(contentSource != NULL)
		{
			mContentSource.reset( contentSource );
		}
	}
	FileStorage * getContentSource() { return mContentSource.get(); }

	void setExtension(const std::string& extension) { mExtension = extension; }
	std::string getExtension() { return mExtension; }

	void setMapFiles(bool flag) { mMapFiles = flag; }
	bool doesMapFiles() { return mMapFiles; }

public:

	_ID	addNew(const std::string& fileName, _TResource * obj)
	{
		obj->setChanged();
		obj->incrUse();
		return add_internal(fileName, obj);
	}

	Data * getResourceData(const std::string& fileName)
	{
		Data * resourceData = NULL;
		if(mContentSource.get() != NULL)
		{
			if(mMapFiles && mContentSource.get()->supportsMappedFiles())
				resourceData = mContentSource.get()->getMappedFile( fileName );
			else
				resourceData = mContentSource.get()->getFile( fileName );
		}
		else
		{
			resourceData = new Data( fileName.c_str(), mMapFiles, true );
		}
		if(resourceData != NULL && !resourceData->isOk())
		{
			DELETE_PTR(resourceData);
			return NULL;
		}
		return resourceData;
	}

	_ID addRetId(const std::string& fileName)
	{
		_TResource * obj = add( fileName );
		return obj != NULL ? obj->getID() : _INVALID_ID;
	}

	_TResource * add(const std::string& fileName)
	{
		return add_internal(fileName, this);
	}

	_TResource * getByName(const std::string& name)
	{
		for(_ID i = 0; i < IDMap<_TResource*>::getSize(); ++i)
		{
			_TResource * obj = IDMap<_TResource*>::get(i);
			if(obj)
			{
				if( obj->getName()==name )
				{
					return obj;
				}
			}
		}
		return NULL;
	}

	_TResource * getByID(_ID id)
	{
		return IDMap<_TResource*>::get( id );
	}

	void release(_ID objID)
	{
		_TResource * obj = IDMap<_TResource*>::get(objID);
		if( obj == NULL ) return;
		if( obj->decrUse() <= 0 )
		{
			mDirty = true;
		}
	}

	//"garbage collection" :)
	void cleanUp()
	{
		if(!mDirty) return;

		for(_ID i = 0; i < IDMap<_TResource*>::getSize(); ++i)
		{
			_TResource * obj = IDMap<_TResource*>::get(i);
			if(obj)
			{
				if( obj->getUse() <= 0 )
				{
					delete obj;
					IDMap<_TResource*>::del(i--);
				}
			}
		}

		mDirty = false;
	}

	int saveChangedResources()
	{
		int saved = 0;
		for(_ID i = 0; i < IDMap<_TResource*>::getSize(); ++i)
		{
			_TResource * obj = IDMap<_TResource*>::get(i);
			if(obj)
			{
				if( obj->isChanged() )
				{
					if(save(obj))
					{
						++saved;
						obj->setChanged(false);
					}
				}
			}
		}

		mDirty = false;

		return saved;
	}

	bool save(_ID id) 
	{ 
		_TResource * resource = IDMap<_TResource*>::get(id);
		if(resource == NULL) return false;

		return save(resource);
	}

protected:

	virtual _TResource* load(Data * data) = 0;
	virtual bool save(_TResource* resource, Data * data, std::string& fileName) { return false; }

	time_t getTimestamp(const std::string& fileName)
	{
		if(mContentSource.get() != NULL)
		{
			return mContentSource->getFileModificationTime(fileName);
		}
		else
		{
			return FileSystem::FileStorage::GetFileModificationTime(fileName.c_str());
		}
	}

	template <class TLoader>
	_TResource * add_internal(const std::string& fileName, TLoader * loader)
	{
		//find resource if existed such one
		_TResource * obj = getByName( fileName );
		if(obj == NULL)
		{
			//if inexisted then load it and add to map
			Data * resourceData = getResourceData(fileName);
			if(resourceData == NULL)
			{
				return NULL;
			}
			obj = loader->load( resourceData );
			//DELETE_PTR( resourceData );
			//some resources (e.g. streaming sound) need to keep resourceData during whole it's life,
			//so such resources must care about releasing resourceData memory.
			//Otherwise load method of derived storage must care about releasing resourceData memory.
			if(obj == NULL)
				return NULL;
			add_internal( fileName, obj );
		}
		obj->incrUse();
		return obj;
	}

	_ID	add_internal(const std::string& fileName, _TResource * obj)
	{
		obj->setName(fileName);
		obj->setID( IDMap<_TResource*>::add(obj) );
		obj->setTimestamp( getTimestamp(fileName) );
		return obj->getID();
	}

private:

	bool save(_TResource * resource) 
	{ 
		Data fileData(NULL, (size_t)1024);

		std::string fileName = resource->getName();
		if( !save(resource, &fileData, fileName) )
		{
			return false;
		}

		bool isExist = false;
		if(mContentSource.get() != NULL)
		{
			isExist = mContentSource.get()->hasFile( fileName );
		}
		else
		{
			isExist = FileStorage::IsFileExist( fileName.c_str() );
		}

		if(isExist && !mAllowOverwriting)
		{
			return false;
		}

		if(mContentSource.get() != NULL)
		{
			if(mContentSource.get()->putFile(&fileData, fileName))
			{
				return true;
			}
		}
		else
		{
			if(fileData.writeToFile( fileName.c_str() ) > 0)
			{
				return true;
			}
		}

		Log::Instance().streamError("ResourceStorage::save") << "Failed to save resource " << fileName.c_str();
		Log::Instance().flush();

		return false; 
	}

};

}//namespace Resource { 
}//namespace Squirrel {