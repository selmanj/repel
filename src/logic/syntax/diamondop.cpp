#include "diamondop.h"

const std::set<Interval::INTERVAL_RELATION>& DiamondOp::defaultRelations() {
	static std::set<Interval::INTERVAL_RELATION>* defaults = new std::set<Interval::INTERVAL_RELATION>();
	if (defaults->size() == 0) {
		defaults->insert(Interval::STARTS);
		defaults->insert(Interval::STARTSI);
		defaults->insert(Interval::DURING);
		defaults->insert(Interval::DURINGI);
		defaults->insert(Interval::FINISHES);
		defaults->insert(Interval::FINISHESI);
		defaults->insert(Interval::OVERLAPS);
		defaults->insert(Interval::OVERLAPSI);
	}
	return *defaults;
}

void DiamondOp::doToString(std::stringstream& str) const {
	str << "<>";
	if (rels_ != DiamondOp::defaultRelations() || !tqconstraints_.empty()) {
		str << "{";
		if (!rels_.empty())	{	// better be safe, not sure if this could happen but still...
			std::set<Interval::INTERVAL_RELATION>::const_iterator it = rels_.begin();
			str << Interval::relationToString(*it);
			it++;
			for (; it!= rels_.end(); it++) {
				str << ", ";
				str << Interval::relationToString(*it);
			}
		}

		if (!tqconstraints_.empty()) {
			str << ":" << tqconstraints_.toString();
		}

		str << "}";
	}
	str << " ";

	if (s_ != NULL) {
		// check if we need parenthesis
		if (s_->precedence() > precedence()) {
			str << "(";
			str << s_->toString();
			str << ")";
		} else {
			str << s_->toString();
		}
	}
};
