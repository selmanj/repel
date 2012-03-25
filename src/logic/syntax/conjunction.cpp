/*
 * conjunction.cpp
 *
 *  Created on: Dec 19, 2011
 *      Author: joe
 */

#include "conjunction.h"
#include "../domain.h"

Conjunction::Conjunction(Sentence *left,
            Sentence *right,
            Interval::INTERVAL_RELATION rel,
            const std::pair<TQConstraints, TQConstraints>* tqconstraints)
        : left_(left), right_(right), rels_(), tqconstraints_() {
    rels_.insert(rel);
    if (tqconstraints) tqconstraints_ = *tqconstraints;
}

Conjunction::Conjunction(Sentence *left,
        Sentence *right,
        const std::pair<TQConstraints, TQConstraints>* tqconstraints)
        : left_(left), right_(right), rels_(defaultRelations()), tqconstraints_() {
    if (tqconstraints) tqconstraints_ = *tqconstraints;
}

Conjunction::Conjunction(const Conjunction& a)
        : left_((a.left_ == 0 ? 0 : a.left_->clone())),
          right_((a.right_ == 0 ? 0 : a.right_->clone())),
          rels_(a.rels_),
          tqconstraints_(a.tqconstraints_) {

}

Conjunction::~Conjunction() {
    delete left_;
    left_ = 0;
    delete right_;
    right_ = 0;
}

void Conjunction::setLeft(Sentence* s) {
    if (left_ && left_ != s) {
        delete left_;
        left_ = 0;
    }
    left_ = s;
}
void Conjunction::setRight(Sentence* s) {
    if (right_ && right_ != s) {
        delete right_;
        right_ = 0;
    }
    right_ = s;
}


void Conjunction::visit(SentenceVisitor& v) const {
    if (left_) left_->visit(v);
    if (right_) right_->visit(v);

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
    if (left_ == 0) {
        str << "null";
    } else if (left_->precedence() > precedence()) {
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
        if (!rels_.empty()) {   // better be safe, not sure if this could happen but still...
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

    if (right_ == 0) {
        str << "null";
    } else if (right_->precedence() > precedence()) {
        str << "(";
        str << right_->toString();
        str << ")";
    } else {
        str << right_->toString();
    }
}

SISet Conjunction::satisfied(const Model& m, const Domain& d, bool forceLiquid) const {
    if (left_ == 0 || right_ == 0) {
        std::string whichNull;
        if (left_ && right_) {
            whichNull = "left_, right_ both 0";
        } else if (left_ && !right_) {
            whichNull = "left_ is 0";
        } else {
            whichNull = "right_ is 0";
        }
        throw std::logic_error("Null sentences found: "+whichNull);
    }
    SISet leftSat = left_->satisfied(m, d, forceLiquid);
    SISet rightSat = right_->satisfied(m, d, forceLiquid);

    if (forceLiquid) {
        leftSat.setForceLiquid(true);
        rightSat.setForceLiquid(true);
        // conjunction must be intersection
        return intersection(leftSat, rightSat);
    } else {
        leftSat.setForceLiquid(false);
        rightSat.setForceLiquid(false);
    }

    SISet result(false, d.maxInterval());
    for(SISet::const_iterator lIt = leftSat.begin(); lIt != leftSat.end(); lIt++ ) {
        for (SISet::const_iterator rIt = rightSat.begin(); rIt != rightSat.end(); rIt++) {
            for(std::set<Interval::INTERVAL_RELATION>::const_iterator relIt = rels_.begin();
                    relIt != rels_.end();
                    relIt++) {
                result.add(composedOf(*lIt, *rIt, *relIt, d.maxSpanInterval()));
            }
        }
    }

    return result;
}
