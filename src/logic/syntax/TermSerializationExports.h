#ifndef TERMSERIALIZATIONEXPORTS_H
#define TERMSERIALIZATIONEXPORTS_H

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include "Term.h"
#include "Constant.h"
//#include "Variable.h"

BOOST_CLASS_EXPORT(Constant)
//BOOST_CLASS_EXPORT(Variable)

// TODO: delete me?
#endif
