#ifndef SENTENCESERIALIZATIONEXPORTS_H
#define SENTENCESERIALIZATIONEXPORTS_H

#include "Atom.h"
#include "BoolLit.h"
#include "Conjunction.h"
#include "DiamondOp.h"
#include "Disjunction.h"
#include "LiquidOp.h"
#include "Negation.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT(Atom)
BOOST_CLASS_EXPORT(BoolLit)
BOOST_CLASS_EXPORT(Conjunction)
BOOST_CLASS_EXPORT(DiamondOp)
BOOST_CLASS_EXPORT(Disjunction)
BOOST_CLASS_EXPORT(LiquidOp)
BOOST_CLASS_EXPORT(Negation)

#endif
