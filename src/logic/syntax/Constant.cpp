#include "Constant.h"

template <class Archive>
void Constant::serialize(Archive& ar, const unsigned int version) {
    // register that there is no need to call the base class serialize
    boost::serialization::void_cast_register<Constant, Term>(
            static_cast<Constant*>(NULL),
            static_cast<Term*>(NULL)
    );
    ar & name_;
}

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

template void Constant::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void Constant::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
