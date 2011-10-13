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
#include <functional>

template<typename K, typename V, typename C = std::less<K> >
class LRUCache {
public:
	typedef std::pair< K,V > ValuePair;
	typedef std::list< ValuePair > ValueList;
	typedef std::map< K, typename ValueList::iterator, C > ValueMap;

	LRUCache(unsigned int maxCapacity);
	virtual ~LRUCache();

	virtual void insert(K key, V value);
	virtual void insert(ValuePair pair);
	virtual int count(K key) const {return map_.count(key);}
	virtual int size() const {return vals_.size();}
	virtual V get(K key);
	virtual void clear();
	virtual unsigned int capacity() const {return maxCapacity_;}
	virtual void setCapacity(unsigned int maxCapacity);
private:
	ValueList vals_;
	ValueMap map_;
	unsigned int maxCapacity_;
};

template<typename K, typename V, typename C>
LRUCache<K,V,C>::LRUCache(unsigned int maxCapacity)
	: vals_(), map_(), maxCapacity_(maxCapacity){

}

template<typename K, typename V, typename C>
LRUCache<K,V,C>::~LRUCache() {

}

template<typename K, typename V, typename C>
void LRUCache<K,V,C>::insert(K key, V value) {
	insert(ValuePair(key, value));
}

template<typename K, typename V, typename C>
void LRUCache<K,V,C>::insert(ValuePair pair) {
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

template<typename K, typename V, typename C>
V LRUCache<K,V,C>::get(K key) {
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

template<typename K, typename V, typename C>
void LRUCache<K,V,C>::clear() {
	map_.clear();
	vals_.clear();
}

template<typename K, typename V, typename C>
void LRUCache<K,V,C>::setCapacity(unsigned int maxCapacity) {
	if (maxCapacity >= maxCapacity_) {
		// no changes needed, great
		maxCapacity_ = maxCapacity;
		return;
	}
	maxCapacity_ = maxCapacity;
	// throw away least recently used items until we are under capacity
	while (vals_.size() > maxCapacity_) {
		K key = vals_.back().first;
		vals_.pop_back();
		map_.erase(key);
	}

}

#endif /* LRUCACHE_H_ */
