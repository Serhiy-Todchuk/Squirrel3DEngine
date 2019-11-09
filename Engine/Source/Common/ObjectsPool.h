#pragma once

#include <stack>

namespace Squirrel {

template <class T>
class ObjectsPool
{
	typedef std::stack<T *>	OBJ_PTRS_POOL;

	OBJ_PTRS_POOL mPool;

public:
	ObjectsPool()	{}
	~ObjectsPool()
	{
		while(!mPool.empty())
		{
			T * obj = mPool.top();
			if(obj != NULL)
				delete obj;
			mPool.pop();
		}
	}

	T * getObj()
	{
		if(!mPool.empty())
		{
			T * obj = mPool.top();
			mPool.pop();
			return obj;
		}
		else
		{
			return new T();
		}
	}

	void putObj(T * obj)
	{
		mPool.push(obj);
	}

};
	
}//namespace Squirrel {