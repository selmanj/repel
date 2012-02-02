/*
 * conjunction.cpp
 *
 *  Created on: Dec 19, 2011
 *      Author: joe
 */

#include "conjunction.h"

void Conjunction::visit(SentenceVisitor& v) const {
	left_->visit(v);
	right_->visit(v);

	v.accept(*this);
}

const std::set<Interval::INTERVAL_RELATION>& Conjunction::defaultRelations() {
	static std::set<Interval::INTERVAL_RELATION>* defaults = new std::set<Interval::INTERVAL_RELATION>();
	if (defaults->empty()) {
		defaults->insert(Interval::EQUALS);
	}
	return *defaults;
}

void Conjunction::doToString(std::stringstream& str) const {
	if (left_->precedence() > precedence()) {
		str << "(";
		str << left_->toString();
		str << ")";
	} else {
		str << left_->toString();
	}
	if (rels_ == defaultRelations() && tqconstraints_.first.empty() && tqconstraints_.second.empty()) {
		str << " ^ ";
	} else if (rels_.size() == 1 && rels_.find(Interval::MEETS) != rels_.end() && tqconstraints_.first.empty() && tqconstraints_.second.empty()) {
		str << " ; ";
	} else {
		str << " ^{";
		if (!rels_.empty())	{	// better be safe, not sure if this could happen but still...
			std::set<Interval::INTERVAL_RELATION>::const_iterator it = rels_.begin();
			str << relationToString(*it);
			it++;
			for (; it!= rels_.end(); it++) {
				str << ", ";
				str << relationToString(*it);
			}
		}
		if (!tqconstraints_.first.empty() || !tqconstraints_.second.empty()) {
			str << ":<";
			if (tqconstraints_.first.empty()) {
				str << "*";
			} else {
				str << tqconstraints_.first.toString();
			}
			str << ", ";
			if (tqconstraints_.second.empty()) {
				str << "*";
			} else {
				str << tqconstraints_.second.toString();
			}
			str << ">";
		}

		str << "} ";
	}

	if (right_->precedence() > precedence()) {
		str << "(";
		str << right_->toString();
		str << ")";
	} else {
		str << right_->toString();
	}
}
