// DestructionPool.h: interface for the DestructionPool class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <stack>
#include <memory>
#include "types.h"
#include "macros.h"

namespace Squirrel {

class DestructionPool
{
private://nested types
	
	class DestroyFunctor
	{
	public:
		virtual ~DestroyFunctor() {}
		virtual void destroy() = 0;
		virtual void * objAddr() = 0;
	};
	
	template <class _T>
	class DestructorWrapper:
	public DestroyFunctor
	{
		_T * mObject;
		void (*mDestroyFunc)(_T * obj);
		
	public:
		DestructorWrapper(_T * obj, void (*func)(_T *)): mObject(obj), mDestroyFunc(func) {}
		virtual ~DestructorWrapper() { destroy(); }
		
		// key method
		virtual void destroy() {
			if(mObject == nullptr)
				return;
			if(mDestroyFunc == nullptr)
				return;
			
			(*mDestroyFunc)(mObject);
			
			mObject = nullptr;
			mDestroyFunc = nullptr;
		}
		
		virtual void * objAddr()
		{
			return mObject;
		}
	};

	template <class _T>
	class DeleteWrapper:
		public DestroyFunctor
	{
		_T * mObject;
		
	public:
		DeleteWrapper(_T * obj): mObject(obj) {}
		virtual ~DeleteWrapper() { destroy(); }
		
		// key method
		virtual void destroy() {
			if(mObject == nullptr)
				return;
			
			delete mObject;
			
			mObject = nullptr;
		}
		
		virtual void * objAddr()
		{
			return mObject;
		}
	};
	
	
private:

	typedef std::deque<DestroyFunctor *> DESTROY_LIST;

public:
	DestructionPool() {};
	~DestructionPool()
	{
		//make sure objects destroyed in the reverse order they were added
		while(mDestroyList.size() > 0)
		{
			DELETE_PTR(mDestroyList.back());
			mDestroyList.pop_back();
		}
	};

public:

	template <class _T>
	void addObject(_T * obj, void (*func)(_T *))
	{
		mDestroyList.push_back( new DestructorWrapper<_T>(obj, func) );
	}
	
	template <class _T>
	void addObject(_T * obj)
	{
		mDestroyList.push_back( new DeleteWrapper<_T>(obj) );
	}
	
	void executeDestruction(void * objAddr)
	{
		DESTROY_LIST::iterator it = mDestroyList.begin();
		for(; it != mDestroyList.end(); ++it)
		{
			if((*it)->objAddr() == objAddr)
			{
				DELETE_PTR((*it));
				mDestroyList.erase(it);
				return;
			}
		}
	}
	
private:
	
	DESTROY_LIST mDestroyList;
};

} //namespace Squirrel {