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

    Model();
    Model(const std::vector<FOL::Event>& pairs);
    Model(const boost::unordered_map<Proposition, SISet>& partialModel);
   // Model(const Model& m);
  //  virtual ~Model();

    std::set<Atom, atomcmp> atoms() const;

    bool hasAtom(const Atom& a) const;

    SISet getAtom(const Atom& a) const;
    void setAtom(const Atom& a, const SISet &set);
    void unsetAtom(const Atom& a, const SISet &set);
    void clearAtom(const Atom& a);

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
};

Model subtractModel(const Model& from, const Model& toSubtract);
Model intersectModel(const Model& a, const Model& b);
Model complimentModel(const Model& a, const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval);

// IMPLEMENTATION
inline Model::Model(const boost::unordered_map<Proposition, SISet>& partialModel) {
    for(boost::unordered_map<Proposition, SISet>::const_iterator it = partialModel.begin();
            it != partialModel.end(); it++) {
        if (amap_.count(it->first.atom) == 0) {
            SISet empty(it->second.forceLiquid(), it->second.maxInterval());
            amap_.insert(std::pair<const Atom, SISet>(it->first.atom, empty));
        }
        if (it->first.sign) {
            amap_[it->first.atom].add(it->second);
        } else {
            amap_[it->first.atom].subtract(it->second);
        }
    }
}

inline bool operator==(const Model& l, const Model& r) {return l.amap_ == r.amap_;}
inline bool operator!=(const Model& l, const Model& r) {return !operator==(l, r);}

#endif /* MODEL_H_ */
