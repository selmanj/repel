/*
 * model.h
 *
 *  Created on: Sep 24, 2011
 *      Author: joe
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <map>
#include "el_syntax.h"
#include "../siset.h"

class Model {
public:
	typedef std::map<Atom, SISet, atomcmp>::const_iterator const_iterator;
	typedef std::map<Atom, SISet, atomcmp>::value_type value_type;

	Model();
	Model(const std::vector<FOL::Event>& pairs);
	Model(const Model& m);
	virtual ~Model();

	std::set<Atom, atomcmp> atoms() const;

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

	bool operator ==(const Model& a) const;
	bool operator !=(const Model& a) const {return !(*this == a);}

	typedef std::map<Atom, SISet, atomcmp> atom_map;

private:

	atom_map amap_;
};

Model subtractModel(const Model& from, const Model& toSubtract);
Model intersectModel(const Model& a, const Model& b);
Model complimentModel(const Model& a, const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval);


#endif /* MODEL_H_ */
