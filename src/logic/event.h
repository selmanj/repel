/*
 * event.h
 *
 *  Created on: Sep 26, 2011
 *      Author: joe
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <boost/shared_ptr.hpp>
#include "el_syntax.h"
#include "../spaninterval.h"

namespace FOL {

// TODO: this is now replaced by wsentence

class Event {
public:
	Event(boost::shared_ptr<Atom> atom, const SpanInterval& where, bool truthVal)
	: atom_(atom), where_(where), truthVal_(truthVal) {};
	~Event() {}

	boost::shared_ptr<Atom> atom() const {return atom_;}
	SpanInterval where() const {return where_;}
	bool truthVal() const {return truthVal_;}

	void setAtom(boost::shared_ptr<Atom> atom) {atom_ = atom;}
	void setWhere(const SpanInterval& si) {where_ = si;}
	void setTruthVal(bool value) {truthVal_ = value;}
private:
	boost::shared_ptr<Atom> atom_;
	SpanInterval where_;
	bool truthVal_;
};
}
#endif /* EVENT_H_ */
