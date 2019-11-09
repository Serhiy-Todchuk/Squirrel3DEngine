// DataMap.h: interface for the Trie class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "HashString.h"
#include <list>
#include <utility>

// (http://habrahabr.ru/post/111874/)

class TrieUtils
{
public:
	
	typedef unsigned char TNodeKey;
	
	template <class TKeyContainer>
	static size_t GetKeysNum(const TKeyContainer& key)
	{
		return sizeof(TKeyContainer);
	}
	
	template <class TKeyContainer>
	static const TNodeKey* GetNodeKeys(const TKeyContainer& key)
	{
		return reinterpret_cast<const TNodeKey*>(&key);
	}
};

template <>
inline size_t TrieUtils::GetKeysNum<std::string>(const std::string& key)
{
	return key.length();
}

template <>
inline const TrieUtils::TNodeKey* TrieUtils::GetNodeKeys<std::string>(const std::string& key)
{
	return reinterpret_cast<const TNodeKey*>(key.c_str());
}

template <>
inline size_t TrieUtils::GetKeysNum<HashString>(const HashString& key)
{
	return key.str().length();
}

template <>
inline const TrieUtils::TNodeKey* TrieUtils::GetNodeKeys<HashString>(const HashString& key)
{
	return reinterpret_cast<const TNodeKey*>(key.c_str());
}

template <class TKey, class TValue>
class Trie
{
	static const int ChildrenPerNode = 256;//std::numeric_limits<TNodeKey>::max()

public:
	
	typedef TrieUtils::TNodeKey TNodeKey;

	struct Node
	{
	public:

		TNodeKey _key;

		TKey	first;
		TValue	second;

	private:

		friend class Trie;

		bool _hasValue;

		Node * _children[ChildrenPerNode];
		int _childrenNum;
		Node * _parent;
		int _level;

		inline Node()
		{
			_level			= 0;
			_childrenNum	= 0;
			_parent			= NULL;
			_hasValue		= false;

			for(int i = 0; i < ChildrenPerNode; ++i)//TODO: replace with memset
				_children[i] = NULL;
		}

		inline ~Node()
		{
			clear();
		}

		inline void clear()
		{
			if(_childrenNum > 0)
				for(int i = 0; i < ChildrenPerNode; ++i)
					if(_children[i] != NULL)
					{
						delete _children[i];
						_children[i] = NULL;
					}
			_childrenNum = 0;
		}

		inline Node * insert(const TNodeKey * keys, size_t keysNum, const TValue& value, const TKey& fullKey)
		{
			if(keysNum == 0)
			{
				first = fullKey;
				second = value;
				_hasValue = true;
				return this;
			}

			const TNodeKey& key = keys[0];
			Node * targetChild = _children[key];

			if(targetChild == NULL)
			{
				targetChild	= new Node();
				_children[key] = targetChild;
				targetChild->_parent = this;
				targetChild->_level = _level + 1;
				targetChild->_key = key;
				++_childrenNum;
			}

			return targetChild->insert(keys + 1, keysNum - 1, value, fullKey);
		}

		inline bool remove(const TNodeKey * keys, int keysNum)
		{
			Node * nodeToRemove = find(keys, keysNum);

			return remove(nodeToRemove);
		}

		inline bool remove(Node * nodeToRemove)
		{
			if(nodeToRemove != NULL)
			{
				if(nodeToRemove->_hasValue)
				{
					//mark as with no value
					nodeToRemove->_hasValue = false;

					if(nodeToRemove->_childrenNum == 0 && nodeToRemove->_parent != NULL)
					{
						//remove node from parent
						nodeToRemove->_parent->_children[nodeToRemove->_key] = NULL;
						--nodeToRemove->_parent->_childrenNum;
						//free memory
						delete nodeToRemove;
					}

					return true;
				}
			}

			return false;
		}

		inline Node * find(const TNodeKey * keys, size_t keysNum)
		{
			if(keysNum == 0)
			{
				return this;
			}

			const TNodeKey& key = keys[0];
			Node * targetChild = _children[key];

			if(targetChild == NULL)
			{
				return NULL;
			}

			return targetChild->find(keys + 1, keysNum - 1);
		}

		inline const Node * find(const TNodeKey * keys, size_t keysNum) const
		{
			if(keysNum == 0)
			{
				return this;
			}

			const TNodeKey& key = keys[0];
			Node * targetChild = _children[key];

			if(targetChild == NULL)
			{
				return NULL;
			}

			return targetChild->find(keys + 1, keysNum - 1);
		}

		inline const Node * nextNotEmpty(const TNodeKey& startKey = 0) const
		{
			if(_childrenNum > 0)
			{
				for(int i = startKey; i < ChildrenPerNode; ++i)
				{
					if(_children[i] == NULL)
						continue;

					if(_children[i]->_hasValue)
						return _children[i];

					return _children[i]->nextNotEmpty();
				}
			}

			if(_parent != NULL && _key < ChildrenPerNode - 2)
			{
				return _parent->nextNotEmpty(_key + 1);
			}

			return NULL;
		}

		inline Node * nextNotEmpty(const TNodeKey& startKey = 0)
		{
			if(_childrenNum > 0)
			{
				for(int i = startKey; i < ChildrenPerNode; ++i)
				{
					if(_children[i] == NULL)
						continue;

					if(_children[i]->_hasValue)
						return _children[i];

					return _children[i]->nextNotEmpty();
				}
			}

			if(_parent != NULL && _key < ChildrenPerNode - 2)
			{
				return _parent->nextNotEmpty(_key + 1);
			}

			return NULL;
		}
	};

	typedef std::list<Node *> NODES_LIST;

	struct const_iterator
	{
		inline const_iterator(): node(NULL) {}
		inline const_iterator(const Node * n) { node = n; }

		const Node * node;

		inline const TValue& value() const
		{
			return node->_value;
		}
		inline const_iterator& operator++()
		{
			node = node->nextNotEmpty();
			return (*this);
		}
		inline bool operator==(const const_iterator& it)
		{
			return node == it.node;
		}
		inline bool operator!=(const const_iterator& it)
		{
			return !operator==(it);
		}
		inline const Node * operator->() const
		{
			return node;
		}
	};

	struct iterator: 
		public const_iterator
	{
		inline iterator() {}
		inline iterator(Node * n): const_iterator(n) {}

		inline iterator& operator++()
		{
			this->node = this->node->nextNotEmpty();
			return (*this);
		}
		inline bool operator==(const iterator& it)
		{
			return this->node == it.node;
		}
		inline bool operator!=(const iterator& it)
		{
			return !operator==(it);
		}
		inline Node * operator->() const
		{
			return (Node *)this->node;
		}
	};


public:
	Trie()	{}
	~Trie()	{}

	inline iterator begin()	{
		return iterator(rootNode.nextNotEmpty());
	}

	inline iterator end()	{
		return iterator(NULL);
	}

	inline const_iterator begin() const	{
		return const_iterator(rootNode.nextNotEmpty());
	}

	inline const_iterator end() const	{
		return const_iterator(NULL);
	}

	inline iterator find(const TKey& key)
	{
		return iterator(findNode(key));
	}

	inline const_iterator find(const TKey& key) const
	{
		return const_iterator(findNode(key));
	}

	inline void erase(iterator position)
	{
		if(position.node != NULL && position.node->_hasValue)
			remove(position.node->first);
	}

	inline void erase(iterator first, iterator last)
	{
		for(iterator curr = first; curr.node != NULL && curr != last;)
		{
			iterator itToEarse = curr;
			++curr;
			remove(itToEarse.node->key);
		}
	}

	inline size_t erase(const TKey& key)
	{
		return remove(key) ? 1 : 0;
	}

	inline size_t size() const
	{
		return nodesWithValues.size();
	}

	inline const NODES_LIST& getNodesWithValues() const { return nodesWithValues; }

	inline Node * insert(const TKey& key, const TValue& value)
	{
		size_t keysNum = TrieUtils::GetKeysNum<TKey>(key);
		const TNodeKey* keys = TrieUtils::GetNodeKeys<TKey>(key);

		Node * node = rootNode.insert(keys, keysNum, value, key);

		if(node != NULL)
		{
			nodesWithValues.push_back(node);
		}

		return node;
	}

	inline bool remove(const TKey& key)
	{
		int keysNum = TrieUtils::GetKeysNum<TKey>(key);
		const TNodeKey* keys = TrieUtils::GetNodeKeys<TKey>(key);

		Node * node = rootNode.find(keys, keysNum);

		if(node != NULL)
		{
			for(typename NODES_LIST::iterator it = nodesWithValues.begin(); it != nodesWithValues.end(); ++it)
			{
				if((*it) == node)
				{
					nodesWithValues.erase(it);
					break;
				}
			}

			return rootNode.remove(node);
		}

		return false;
	}

	inline bool contains(const TKey& key) const
	{
		size_t keysNum = TrieUtils::GetKeysNum<TKey>(key);
		const TNodeKey* keys = TrieUtils::GetNodeKeys<TKey>(key);

		const Node * node = rootNode.find(keys, keysNum);

		return (node != NULL) && node->_hasValue;
	}

	inline Node * findNode(const TKey& key)
	{
		size_t keysNum = TrieUtils::GetKeysNum<TKey>(key);
		const TNodeKey* keys = TrieUtils::GetNodeKeys<TKey>(key);

		Node * node = rootNode.find(keys, keysNum);

		if((node != NULL) && node->_hasValue)
			return node;

		return NULL;
	}

	inline const Node * findNode(const TKey& key) const
	{
		size_t keysNum = TrieUtils::GetKeysNum<TKey>(key);
		const TNodeKey* keys = TrieUtils::GetNodeKeys<TKey>(key);

		const Node * node = rootNode.find(keys, keysNum);

		if((node != NULL) && node->_hasValue)
			return node;

		return NULL;
	}

	inline void clear()
	{
		rootNode.clear();
	}

	inline TValue& operator[](const TKey& key)
	{
		Node * node = findNode(key);

		if(node == NULL)
		{
			node = insert(key, TValue());
		}

		return node->second;
	}
	
private:
	
	NODES_LIST nodesWithValues;

	Node rootNode;
};

