/*
 * MCSatSampleStrategySerializationExport.h
 *
 *  Created on: May 27, 2012
 *      Author: joe
 */

#ifndef MCSATSAMPLESTRATEGYSERIALIZATIONEXPORT_H_
#define MCSATSAMPLESTRATEGYSERIALIZATIONEXPORT_H_

#include "MCSatSampleStrategy.h"
#include "MCSatSampleLiquidlyStrategy.h"
#include "MCSatSamplePerfectlyStrategy.h"
#include <boost/serialization/export.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

BOOST_CLASS_EXPORT(MCSatSampleLiquidlyStrategy)
BOOST_CLASS_EXPORT(MCSatSamplePerfectlyStrategy)

#endif /* MCSATSAMPLESTRATEGYSERIALIZATIONEXPORT_H_ */
