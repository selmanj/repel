/*
 * model.h
 *
 *  Created on: Sep 24, 2011
 *      Author: joe
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <boost/unordered_map.hpp>
#include <utility>
//#include "el_syntax.h"
#include "../siset.h"
#include "syntax/atom.h"
#include "event.h"

class Model {
public:
    typedef boost::unordered_map<Atom, SISet>::const_iterator const_iterator;
    typedef boost::unordered_map<Atom, SISet>::value_type value_type;

    Model(const Interval& maxInterval_);
    Model(const std::vector<FOL::Event>& pairs, const Interval& maxInterval_);
    Model(const boost::unordered_map<Proposition, SISet>& partialModel, const Interval& maxInterval_);
   // Model(const Model& m);
  //  virtual ~Model();

    friend std::size_t hash_value(const Model& m);

    std::set<Atom, atomcmp> atoms() const;

    bool hasAtom(const Atom& a) const;

    SISet getAtom(const Atom& a) const;
    void setAtom(const Atom& a, const SISet &set);
    void unsetAtom(const Atom& a, const SISet &set);
    void clearAtom(const Atom& a);

    Interval maxInterval() const;
    void setMaxInterval(const Interval& maxInterval);

    void subtract(const Model& toSubtract);
    void intersect(const Model& b);
    void compliment(const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval);

    unsigned long size() const;
    void swap(Model& b) { amap_.swap(b.amap_); };

    std::string toString() const;

    /*
    bool operator ==(const Model& a) const;
    bool operator !=(const Model& a) const {return !(*this == a);}
*/
    friend bool operator==(const Model& l, const Model& r);
    friend bool operator!=(const Model& l, const Model& r);

    Model& operator=(const Model m) { if (*this != m) amap_ = m.amap_; return *this;}
    typedef boost::unordered_map<Atom, SISet> atom_map;

private:

    atom_map amap_;
    Interval maxInterval_;
};

Model subtractModel(const Model& from, const Model& toSubtract);
Model intersectModel(const Model& a, const Model& b);
Model complimentModel(const Model& a, const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval);

// IMPLEMENTATION
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

#endif /* MODEL_H_ */
