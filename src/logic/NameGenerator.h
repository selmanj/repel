/*
 * NameGenerator.h
 *
 *  Created on: Jun 27, 2011
 *      Author: joe
 */

#ifndef NAMEGENERATOR_H_
#define NAMEGENERATOR_H_
#include <string>
#include <boost/serialization/access.hpp>

#define NAME_PREFIX "__anonPred"

class NameGenerator {
public:
    NameGenerator();
    std::string getUniqueName();

    friend bool operator==(const NameGenerator& l, const NameGenerator& r);
    friend bool operator!=(const NameGenerator& l, const NameGenerator& r);
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

    unsigned int counter_;
};

template <class Archive>
void NameGenerator::serialize(Archive& ar, const unsigned int version) {
    ar & counter_;
}

inline bool operator==(const NameGenerator& l, const NameGenerator& r) { return l.counter_ == r.counter_;}
inline bool operator!=(const NameGenerator& l, const NameGenerator& r) { return !operator==(l,r);}
#endif /* NAMEGENERATOR_H_ */
