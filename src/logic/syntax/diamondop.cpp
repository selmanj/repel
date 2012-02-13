#include "diamondop.h"
#include "../domain.h"

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
        if (!rels_.empty()) {   // better be safe, not sure if this could happen but still...
            std::set<Interval::INTERVAL_RELATION>::const_iterator it = rels_.begin();
            str << relationToString(*it);
            it++;
            for (; it!= rels_.end(); it++) {
                str << ", ";
                str << relationToString(*it);
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

SISet DiamondOp::doSatisfied(const Model& m, const Domain& d, bool forceLiquid) const {
    if (forceLiquid) throw std::runtime_error("DiamondOp::doSatisfied(): given parameter forceLiquid=true, but diamond op is a non liquid operator!");

    SISet sat = s_->satisfied(m, d, false);

    SISet newsat(false, sat.maxInterval());
    for(SISet::const_iterator sIt = sat.begin(); sIt != sat.end(); sIt++) {
        for(std::set<Interval::INTERVAL_RELATION>::const_iterator relIt = rels_.begin();
                relIt != rels_.end();
                relIt++) {
            boost::optional<SpanInterval> spr = sIt->satisfiesRelation(*relIt, d.maxSpanInterval());
            if (spr) newsat.add(*spr);
        }
    }

    return newsat;
}
