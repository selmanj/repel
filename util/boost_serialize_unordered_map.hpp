/*
 * boost_serialize_unordered_map.hpp
 *
 *  Created on: May 27, 2012
 *      Author: joe
 */

#ifndef BOOST_SERIALIZE_UNORDERED_MAP_HPP_
#define BOOST_SERIALIZE_UNORDERED_MAP_HPP_

#include <boost/unordered_map.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/nvp.hpp>

namespace boost { namespace serialization {
template<class Archive, typename T, typename U>
inline void serialize(
    Archive & ar,
    boost::unordered_map<T,U> & t,
    const unsigned int file_version
){
    split_free(ar, t, file_version);
}

template<class Archive, typename T, typename U>
void save(Archive & ar, const boost::unordered_map<T,U> & t, unsigned int version)
{
    // write the size
    // TODO: should we handle bucket size as well?
    typedef typename boost::unordered_map<T, U>::size_type size_type;
    typedef typename boost::unordered_map<T,U>::const_iterator const_iterator;
    size_type size = t.size();
    ar & BOOST_SERIALIZATION_NVP(size);
    unsigned int count = 0;
    for (const_iterator it = t.begin(); it != t.end(); it++) {
        ar & boost::serialization::make_nvp("pair" + count, *it);
        count++;
    }
}

template<class Archive, typename T, typename U>
void load(Archive & ar, boost::unordered_map<T,U> & t, unsigned int version)
{
    // clear the map
    t.clear();
    // read the size
    typedef typename boost::unordered_map<T,U>::size_type size_type;
    size_type size;
    ar & BOOST_SERIALIZATION_NVP(size);
    for (size_type i = 0; i < size; i++) {
        // read a pair
        std::pair<T, U> pair;
        ar & boost::serialization::make_nvp("pair"+i, pair);
        // insert it into the map
        t.insert(pair);
    }
}

}}
#endif /* BOOST_SERIALIZE_UNORDERED_MAP_HPP_ */
