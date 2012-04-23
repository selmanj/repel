#ifndef PROPOSITION_H_
#define PROPOSITION_H_

#include "Atom.h"

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
    Atom atom_;
    bool sign_;  // if positive, true, if negative, false
};


#endif
