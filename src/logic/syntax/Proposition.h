#ifndef PROPOSITION_H_
#define PROPOSITION_H_

#include "Atom.h"
#include <boost/serialization/access.hpp>

/**
 * Simple container class for propositions (literals) without a temporal quantifier.
 */
class Proposition {
public:
    Proposition(const Atom& a, bool s);

    Proposition inverse() const;
    friend std::size_t hash_value(const Proposition& p);
    friend bool operator==(const Proposition& l, const Proposition& r);
    friend bool operator!=(const Proposition& l, const Proposition& r);

    Atom atom() const;
    bool sign() const;

    void setAtom(const Atom& a);
    void setSign(bool b);
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);

    Atom atom_;
    bool sign_;  // if positive, true, if negative, false
};

std::ostream& operator<<(std::ostream& out, const Proposition& p);

template <class Archive>
void Proposition::serialize(Archive& ar, const unsigned int version) {
    ar & atom_;
    ar & sign_;
}

#endif
