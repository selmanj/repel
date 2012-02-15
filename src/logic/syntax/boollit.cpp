/*
 * boollit.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: joe
 */

#include "boollit.h"
#include "../domain.h"

bool BoolLit::doEquals(const Sentence& t) const {
    const BoolLit *bt = dynamic_cast<const BoolLit*>(&t);
    if (bt == NULL) {
        return false;
    }

    return bt->val_ == val_;
}

SISet BoolLit::satisfied(const Model& m, const Domain& d, bool forceLiquid) const {
    if (!val_) return SISet(forceLiquid, d.maxInterval());
    return SISet(d.maxSpanInterval(), forceLiquid, d.maxInterval());
}
