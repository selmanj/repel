/*
 * Model.h
 *
 *  Created on: Sep 24, 2011
 *      Author: joe
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <boost/unordered_map.hpp>
#include <utility>
#include <boost/serialization/access.hpp>
#include <boost/serialization/map.hpp>
#include "../util/boost_serialize_unordered_map.hpp"
#include "../SISet.h"
#include "syntax/Atom.h"
#include "syntax/Constant.h"
#include "Event.h"

class Model {
public:
    typedef boost::unordered_map<Atom, SISet>::const_iterator const_iterator;
    typedef boost::unordered_map<Atom, SISet>::value_type value_type;

    Model();
    explicit Model(const Interval& maxInterval_);
    Model(const std::vector<FOL::Event>& pairs, const Interval& maxInterval_);
    Model(const boost::unordered_map<Proposition, SISet>& partialModel, const Interval& maxInterval_);
   // Model(const Model& m);
  //  virtual ~Model();

    friend std::size_t hash_value(const Model& m);

    bool hasAtom(const Atom& a) const;

    SISet getAtom(const Atom& a) const;
    void setAtom(const Atom& a, const SISet &set);
    void unsetAtom(const Atom& a, const SISet &set);
    void clearAtom(const Atom& a);

    Interval maxInterval() const;
    void setMaxInterval(const Interval& maxInterval);

    void subtract(const Model& toSubtract);
    void intersect(const Model& b);

    unsigned long size() const;
    void swap(Model& b) { amap_.swap(b.amap_); };
    std::string toString() const;
    /*
    bool operator ==(const Model& a) const;
    bool operator !=(const Model& a) const {return !(*this == a);}
*/
    friend bool operator==(const Model& l, const Model& r);
    friend bool operator!=(const Model& l, const Model& r);

    friend std::ostream& operator<<(std::ostream& out, const Model& m);
    Model& operator=(const Model& m) { if (this != &m) amap_ = m.amap_; return *this;}   // TODO: use swap() dogg


private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
/*
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const;
    template <class Archive>
    void load(Archive& ar, const unsigned int version);
    BOOST_SERIALIZATION_SPLIT_MEMBER()
*/
    typedef boost::unordered_map<Atom, SISet> atom_map;

    atom_map amap_;
    Interval maxInterval_;
};

// IMPLEMENTATION
inline Model::Model()
    : amap_(), maxInterval_(0,0) {}
inline Model::Model(const Interval& maxInterval)
    : amap_(), maxInterval_(maxInterval) {}


inline Interval Model::maxInterval() const {return maxInterval_;}

inline bool operator==(const Model& l, const Model& r) {return l.amap_ == r.amap_ && l.maxInterval_ == r.maxInterval_;}
inline bool operator!=(const Model& l, const Model& r) {return !operator==(l, r);}

inline std::size_t hash_value(const Model& m) {
    std::size_t seed = 0;
    boost::hash_range(seed, m.amap_.begin(), m.amap_.end());
    boost::hash_combine(seed, m.maxInterval_);
    return seed;
}

template <class Archive>
void Model::serialize(Archive& ar, const unsigned int version) {
    ar & amap_;
    ar & maxInterval_;
}
/*
template <class Archive>
void Model::save(Archive& ar, const unsigned int version) const {
    // no support for boost-serialization and the unordered map, so we'll
    // do it by hand.  this isn't an exact serialization but should be
    // enough for our purposes

    // write the size
    // TODO: should we handle bucket size as well?
    boost::unordered_map<Atom, SISet>::size_type size = amap_.size();
    ar & size;
    for (boost::unordered_map<Atom, SISet>::const_iterator it = amap_.begin(); it != amap_.end(); it++) {
        ar & *it;
    }
    // max interval
    ar & maxInterval_;
}

template <class Archive>
void Model::load(Archive& ar, const unsigned int version) {
    // clear the map
    amap_.clear();
    // read the size
    boost::unordered_map<Atom, SISet>::size_type size;
    ar & size;
    for (boost::unordered_map<Atom, SISet>::size_type i = 0; i < size; i++) {
        // read a pair
        std::pair<Atom, SISet> pair;
        ar & pair;
        // insert it into the map
        amap_.insert(pair);
    }
    ar & maxInterval_;
}
*/

#endif /* MODEL_H_ */
