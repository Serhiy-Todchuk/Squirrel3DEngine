#pragma once

#include "types.h"
#include <vector>

namespace Squirrel {

template<class _T> class IDMap
{
	std::vector<_T> map;
	int count;
	_ID firstFree;
	int capacityIncr;
	_T nullValue;

public:
	//constr/destr
	IDMap(void)
	{
		count = 0;
		firstFree = 0;
		capacityIncr = 128;
	}
	virtual ~IDMap(void)
	{
		map.clear();
	}
	inline int	getCount(void)	{	return count;	}
	inline int	getSize(void)	{	return (int)map.size();	}

	inline int	getCapacityIncr(void)	{	return capacityIncr;	}
	inline void	setCapacityIncr(int c)	{	capacityIncr = c;	}

	inline _T	getNullValue(void)		{	return nullValue;	}
	inline void	setNullValue(_T value)	{	nullValue = value;	}

	//replace (set) elem with other elem
	void	set(_ID id, _T obj)
	{
		if(id >= (_ID)map.size())
		{
			map.reserve( map.size() + capacityIncr );
			map.resize( id + 1 );
		}

		if(!map[id])//if target cell is empty
		{
			if(obj)//if new object is existed	
			{
				++count;
				if(id == firstFree)
				{
					++firstFree;
				}
			}
		}
		else//if target cell is busy
		{
			if(!obj)
			{
				--count;
			}
		}

		map[id] = obj;
	}

	//returns object's id in this map if object is existed here
	_ID	getId(_T obj)
	{
		if(obj)
		{
			for(_ID i = 0; i < getSize(); i++)
			{
				if(map[i] == obj)
				{
					return i;
				}
			}
		}
		return -1;
	}

	//add elem
	_ID	add(_T obj)
	{
		if((unsigned)firstFree >= map.size())
		{
			//map is full
			map.reserve( map.size() + capacityIncr );
			map.push_back(obj);
			firstFree = (_ID)map.size();
			++count;
			return (_ID)map.size()-1;
		}
		else
		{
			if(!map[firstFree])//firstFree cell is empty
			{
				//printf("free cell - OK\n");
				map[firstFree] = obj;
				++firstFree;
				++count;				
				return firstFree;
			}
			else//finding free cell
			{
				//printf("searching for free cell\n");
				for(_ID i = firstFree+1; (unsigned)i < map.size(); i++)
					if(!map[i])
					{
						//printf("free cell found\n");
						map[i] = obj;
						firstFree = i+1;
						++count;
						return i;
					}
				//printf("free cell not found\n");
				//if not found then pushBack
				map.push_back(obj);
				firstFree = (_ID)map.size();
				++count;
				return (_ID)map.size()-1;
			}
		}
		return -1;
	}
	//delete elem
	void	del(_ID objID)
	{
		if((unsigned)objID >= map.size()) return;
		if(!map[objID]) return;
		map[ objID ] = 0;
		if(firstFree > objID) firstFree = objID;
		--count;
	}
	//return (get) elem from map
	inline _T		get(_ID objID)
	{
		return ((unsigned)objID >= map.size())? 0 : map[objID];
	}
	void clear()
	{
		map.clear();
		count = 0;
		firstFree = 0;
	}
};

}//namespace Squirrel {
