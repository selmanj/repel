/*
 * LRUCache.h
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

template<typename K, typename V, typename C>
class LRUCache; // forward declaration
template<typename K, typename V, typename C, typename A>
class LRUCacheIterator; // forward declaration

template<typename K, typename V, typename C = std::less<K> >
class LRUCache {
private:
    typedef std::pair< K,V > ValuePair;
    typedef std::list< ValuePair > ValueList;
    typedef std::map< K, typename ValueList::iterator, C > ValueMap;
public:
    typedef K key_type;
    typedef V mapped_type;
    typedef std::pair<K, V> value_type;
    typedef C key_compare;

    class value_compare : public std::binary_function<value_type, value_type, bool> {
        friend class LRUCache<K, V, C>;
    protected:
        C comp_;
        value_compare(C comp__) : comp_(comp__) {}
    public:
        bool operator()(const value_type& x, const value_type& y) {
            return comp_(x.first, y.first);
        }
    };

    typedef LRUCacheIterator<K, ValuePair, C, typename ValueMap::allocator_type> iterator;

    LRUCache(unsigned int maxCapacity=1024);

    void insert(K key, V value);
    void insert(const std::pair<K, V>& pair);
    int count(K key) const {return map_.count(key);}
    int size() const {return vals_.size();}
    V get(K key);
    void clear();
    unsigned int capacity() const {return maxCapacity_;}
    void setCapacity(unsigned int maxCapacity);
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
void LRUCache<K,V,C>::insert(K key, V value) {
    insert(ValuePair(key, value));
}

template<typename K, typename V, typename C>
void LRUCache<K,V,C>::insert(const std::pair<K, V>& pair) {
    if (maxCapacity_ == 0) return;
    // check to see if we have something already, if so overwrite
    K key = pair.first;
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

namespace {
template <class K, class V, class C, class A>
class LRUCacheIterator : std::iterator<std::bidirectional_iterator_tag, V> {
public:
    LRUCacheIterator() : it_() {}
    LRUCacheIterator(const LRUCacheIterator& i) : it_(i.it_) {}

    LRUCacheIterator& operator=(const LRUCacheIterator& i) {
        if (this != &i) {
            it_ = i.it_;
        }
        return *this;
    }

    V& operator*() {
        return *(it_->second);
    }

    V* operator->() {
        return &(operator*());
    }

    LRUCacheIterator& operator++() {
        it_++;
        return *this;
    }

    LRUCacheIterator operator++(int) {
        LRUCacheIterator itold(*this);
        it_++;
        return itold;
    }

    LRUCacheIterator& operator--() {
        it_--;
        return *this;
    }

    LRUCacheIterator operator--(int) {
        LRUCacheIterator itold(*this);
        it_--;
        return itold;
    }

    bool operator==(const LRUCacheIterator& i) const{
        return (it_ == i.it_);
    }
    bool operator!=(const LRUCacheIterator& i) const {return !operator==(i);}
protected:
    typedef typename std::map<K, typename std::list<V>::iterator, C, A >::iterator cache_it;
    LRUCacheIterator(const cache_it& it) : it_(it) {}
private:
    cache_it it_;
};
}

#endif /* LRUCACHE_H_ */
