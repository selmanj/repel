
#include "proposition.h"
#include "atom.h"

Proposition::Proposition(const Atom& a, bool s)
    : atom_(a), sign_(s) {
    if (!atom_.isGrounded()) throw std::invalid_argument("Cannot initialize a Proposition with an atom containing variables.");
}

Proposition Proposition::inverse() const {
    return Proposition(atom_, !sign_);
}

bool operator==(const Proposition& l, const Proposition& r) { return l.sign_ == r.sign_ && l.atom_ == r.atom_; }
bool operator!=(const Proposition& l, const Proposition& r) { return !operator==(l,r);}

std::size_t hash_value(const Proposition& p) {
    std::size_t seed = 0;
    boost::hash_combine(seed, p.atom_);
    boost::hash_combine(seed, p.sign_);
    return seed;
}

Atom Proposition::atom() const {
    return atom_;
}
bool Proposition::sign() const {
    return sign_;
}

void Proposition::setAtom(const Atom& a) {
    atom_ = a;
}
void Proposition::setSign(bool b) {
    sign_ = b;
}
