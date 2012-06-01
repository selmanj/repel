/*
 * AllSerializationExports.h
 *
 *  Created on: May 27, 2012
 *      Author: joe
 */

#ifndef ALLSERIALIZATIONEXPORTS_H_
#define ALLSERIALIZATIONEXPORTS_H_

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>

#include "logic/syntax/Atom.h"
#include "logic/syntax/BoolLit.h"
#include "logic/syntax/Conjunction.h"
#include "logic/syntax/Constant.h"
#include "logic/syntax/DiamondOp.h"
#include "logic/syntax/Disjunction.h"
#include "logic/syntax/LiquidOp.h"
#include "logic/syntax/Negation.h"
#include "logic/syntax/Variable.h"
#include "inference/MCSatSamplePerfectlyStrategy.h"
#include "inference/MCSatSampleLiquidlyStrategy.h"

/*
#include "inference/MCSatSampleStrategySerializationExport.h"
#include "logic/syntax/TermSerializationExports.h"
#include "logic/syntax/SentenceSerializationExport.h"
*/
/**
 * Register all the derived types so our archive knows how to handle them.
 * NOTE: I could not get the BOOST_CLASS_EXPORT macros to work at all, so I
 * resorted to this method which seems to work, if a bit inelegant.
 */
template <class Archive>
void registerAllPELTypes(Archive& ar) {
    ar.template register_type<Atom>();
    ar.template register_type<BoolLit>();
    ar.template register_type<Conjunction>();
    ar.template register_type<Constant>();
    ar.template register_type<DiamondOp>();
    ar.template register_type<Disjunction>();
    ar.template register_type<LiquidOp>();
    ar.template register_type<Negation>();
    ar.template register_type<Variable>();
    ar.template register_type<MCSatSamplePerfectlyStrategy>();
    ar.template register_type<MCSatSampleLiquidlyStrategy>();
}




#endif /* ALLSERIALIZATIONEXPORTS_H_ */
