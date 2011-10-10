/*
 * lrucache.h
 *
 *  Created on: Oct 10, 2011
 *      Author: joe
 */

#ifndef LRUCACHE_H_
#define LRUCACHE_H_
#include <list>
#include <map>
#include <iostream>
#include <iterator>

template<typename K, typename V>
class LRUCache {
public:
	typedef std::pair< K,V > ValuePair;
	typedef std::list< ValuePair > ValueList;
	typedef std::map< K, typename ValueList::iterator > ValueMap;

	LRUCache(unsigned int maxCapacity);
	virtual ~LRUCache();

	virtual void insert(ValuePair pair);
	virtual int count(K key) const {return map_.count(key);}
	virtual int size() const {return vals_.size();}
	virtual V get(K key);
private:
	ValueList vals_;
	ValueMap map_;
	unsigned int maxCapacity_;
};

template<typename K, typename V>
LRUCache<K, V>::LRUCache(unsigned int maxCapacity)
	: vals_(), map_(), maxCapacity_(maxCapacity){

}

template<typename K, typename V>
LRUCache<K, V>::~LRUCache() {

}

template<typename K, typename V>
void LRUCache<K, V>::insert(ValuePair pair) {
	// check to see if we have something already, if so overwrite
	K key = pair.first;
	V value = pair.second;
	if (map_.count(key) == 1) {
		typename ValueMap::iterator oldPairIt = map_.find(key);
		typename ValueList::iterator oldVal = oldPairIt->second;
		vals_.erase(oldVal);
		map_.erase(oldPairIt);
	}
	// if we are at capacity, remove the last item (least recently used)
	if (vals_.size() >= maxCapacity_) {
		ValuePair lruItem = vals_.back();
		vals_.pop_back();
		map_.erase(lruItem.first);
	}
	// add the item
	vals_.push_front(pair);
	typename ValueList::iterator valIt = vals_.begin();
	map_.insert(std::pair<K, typename ValueList::iterator>(key, valIt));
}

template<typename K, typename V>
V LRUCache<K,V>::get(K key) {
	typename ValueMap::iterator oldMapIt = map_.find(key);
	typename ValueList::iterator oldValIt = oldMapIt->second;
	ValuePair valPair = *oldValIt;
	if (oldValIt == vals_.begin()) {
		return valPair.second;
	}
	// do some rearranging
	vals_.erase(oldValIt);
	map_.erase(oldMapIt);

	vals_.push_front(valPair);
	typename ValueList::iterator valIt = vals_.begin();
	map_.insert(std::pair<K, typename ValueList::iterator>(key, valIt));
	return valPair.second;
}

#endif /* LRUCACHE_H_ */
