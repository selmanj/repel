/*
 * boost_serialize_unordered_set.hpp
 *
 *  Created on: May 27, 2012
 *      Author: joe
 */

#ifndef BOOST_SERIALIZE_UNORDERED_SET_HPP_
#define BOOST_SERIALIZE_UNORDERED_SET_HPP_

#include <boost/unordered_set.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/nvp.hpp>

namespace boost { namespace serialization {
template<class Archive, typename T>
inline void serialize(
    Archive & ar,
    boost::unordered_set<T> & t,
    const unsigned int file_version
){
    split_free(ar, t, file_version);
}

template<class Archive, typename T>
void save(Archive & ar, const boost::unordered_set<T> & t, unsigned int version)
{
    // write the size
    // TODO: should we handle bucket size as well?
    typedef typename boost::unordered_set<T>::size_type size_type;
    typedef typename boost::unordered_set<T>::const_iterator const_iterator;
    size_type size = t.size();
    ar & BOOST_SERIALIZATION_NVP(size);
    unsigned int count = 0;
    for (const_iterator it = t.begin(); it != t.end(); it++) {
        ar & boost::serialization::make_nvp("item" + count, *it);
        count++;
    }
}

template<class Archive, typename T>
void load(Archive & ar, boost::unordered_set<T> & t, unsigned int version)
{
    // clear the set
    t.clear();
    // read the size
    typedef typename boost::unordered_set<T>::size_type size_type;
    size_type size;
    ar & BOOST_SERIALIZATION_NVP(size);
    for (size_type i = 0; i < size; i++) {
        // read a pair
        T item;
        ar & boost::serialization::make_nvp("item"+i, item);
        // insert it into the set
        t.insert(item);
    }
}

}}
#endif /* BOOST_SERIALIZE_UNORDERED_SET_HPP_ */
