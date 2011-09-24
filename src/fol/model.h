/*
 * model.h
 *
 *  Created on: Sep 24, 2011
 *      Author: joe
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <map>
#include "fol.h"
#include "../siset.h"

struct atomcmp {
	bool operator()(const Atom& a, const Atom& b) const {
		return a.toString() < b.toString();
	}
};

class Model {
public:
	typedef std::map<Atom, SISet, atomcmp>::const_iterator const_iterator;
	typedef std::map<Atom, SISet, atomcmp>::value_type value_type;

	Model();
	Model(const std::vector<FOL::EventPair>& pairs);
	Model(const Model& m);
	virtual ~Model();

	virtual const_iterator begin() const {return amap_.begin();}
	virtual const_iterator end() const {return amap_.end();}

	virtual bool hasAtom(const Atom& a) const;

	virtual SISet getAtom(const Atom& a) const;
	virtual void setAtom(const Atom& a, const SISet &set);
	virtual void unsetAtom(const Atom& a, const SISet &set);
	virtual void clearAtom(const Atom& a);

	virtual void setMaxInterval(const Interval& maxInterval);

	virtual void subtract(const Model& toSubtract);
	virtual void intersect(const Model& b);
	virtual void compliment(const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval);

	virtual unsigned long size() const;
	virtual void swap(Model& b) { amap_.swap(b.amap_); };

	virtual std::string toString() const;

private:
	typedef std::map<Atom, SISet, atomcmp> atom_map;

	atom_map amap_;
};

Model subtractModel(const Model& from, const Model& toSubtract);
Model intersectModel(const Model& a, const Model& b);
Model complimentModel(const Model& a, const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval);


#endif /* MODEL_H_ */
