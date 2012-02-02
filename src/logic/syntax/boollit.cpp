/*
 * boollit.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: joe
 */

#include "boollit.h"

bool BoolLit::doEquals(const Sentence& t) const {
    const BoolLit *bt = dynamic_cast<const BoolLit*>(&t);
    if (bt == NULL) {
        return false;
    }

    return bt->val_ == val_;
}
