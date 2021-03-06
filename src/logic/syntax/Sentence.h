#ifndef SENTENCE_H
#define SENTENCE_H

#include <string>
#include <sstream>
#include <boost/utility.hpp>
#include <boost/functional/hash.hpp>
#include <boost/serialization/access.hpp>
#include "SentenceVisitor.h"
#include "../../SISet.h"

class Domain;
class Model;

/**
 * Abstract base class representing a logical sentence.
 * The Sentence class is the abstract base class representing any sentence in
 * event logic.  All syntactic elements should inherit from it.
 */

class Sentence : boost::noncopyable {
public:
    /**
     * Destructor, declared virtual.
     */
    virtual ~Sentence();

    /**
     * Get a clone (deep copy) of this Sentence.
     * Note that the caller of this function should take ownership of the
     * returned pointer, in order to avoid memory leaks.
     *
     * @return A ptr to a new Sentence object allocated on the heap.
     */
    Sentence* clone() const;

    /**
     * Test for equality via == operator.
     *
     * @param b sentence object to compare to
     * @return true if equal, false otherwise.
     */
    bool operator==(const Sentence& b) const;

    /**
     * Test for inequality via != operator
     *
     * @param b sentence object to compare to
     * @return true if inequal, false otherwise.
     */
    bool operator!=(const Sentence& b) const;

    /**
     * Return this sentence as a string.
     *
     * @return a string representation of the sentence
     */
    std::string toString() const;

    /**
     * Get the sentence's precedence.
     * Precedence is used to determine order of operations.  A higher
     * precedence is always evaluated first.
     *
     * @return an int representing the sentence precedence
     */
    int precedence() const;

    /**
     * Visit this sentence and all descending sentences with the given visitor.
     * Implements the visitor pattern.  Given a SentenceVisitor, this sentence
     * and all sentences contained within will call the SentenceVisitor::accept()
     * method with the current sentence.
     *
     * @param s SentenceVisitor to call back when parsing this sentence
     */
    virtual void visit(SentenceVisitor& s) const = 0;

    /**
     * Check to see if this sentence contains another sentence s.  The sentence
     * is checked via equality, and can match either the current sentence or a
     * descendant. (i.e. "P" is contained in "!(P v Q)".
     *
     * @param s sentence to find (the needle)
     * @return true if the sentence is found, false otherwise
     */
    bool contains(const Sentence& s) const;

    /**
     * Given a model and a domain, return the set of intervals where this
     * sentence is satisfied (where it is true).
     *
     * @param m a model to evaluate the sentence on
     * @param d domain that the sentence and model belong to
     * @param forceLiquid whether or not to force the resulting SISet to be
     *   liquid.  If in doubt, leave it set to false.
     * @param where optional pointer to a SISet specifying where to check the
     * truth value at.
     * @return a SISet containing the intervals where the sentence is true at.
     */
    //virtual SISet satisfied(const Model& m, const Domain& d, bool forceLiquid, const SISet* where) const;

    SISet dSatisfied(const Model& m, const Domain& d) const;
    SISet dSatisfied(const Model& m, const Domain& d, const SISet& where) const;
    SISet dNotSatisfied(const Model& m, const Domain& d) const;
    SISet dNotSatisfied(const Model& m, const Domain& d, const SISet& where) const;
    virtual SISet satisfied(const Model& m, const Domain& d, bool forceLiquid) const = 0;


    /**
     * Get a std::size_t value representing the type.  This value can be
     * compared at runtime to the static value typeCode() on each class to
     * cheaply see what class you are dealing with.
     *
     * @return a std::size_t value uniquely defined for each class
     */
    virtual std::size_t getTypeCode() const = 0;

    friend std::size_t hash_value(const Sentence& s);
private:
    virtual void doToString(std::stringstream& str) const = 0;
    virtual Sentence* doClone() const = 0;
    virtual bool doEquals(const Sentence& t) const = 0;
    virtual int doPrecedence() const = 0;
    virtual bool doContains(const Sentence& s) const = 0;
    virtual std::size_t doHashValue() const = 0;
};

/**
 * A set of constraints applied to a temporal quantifier.  Specifically,
 * constraints are used to restrict the set of intervals a quantifier may
 * be bound to when using either the diamond operator or the conjunction
 * operator.  The constraints are either "must be in set x" or "must not be in
 * set x".
 */
struct TQConstraints {
public:
    SISet mustBeIn;
    SISet mustNotBeIn;

    TQConstraints();
    TQConstraints(const Interval& maxInterval);

    bool operator==(const TQConstraints& b) const;
    bool operator!=(const TQConstraints& b) const;
    std::string toString() const;
    bool empty() const;
private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

bool isDisjunctionOfPELCNFLiterals(const Sentence& sentence);
bool isPELCNFLiteral(const Sentence& sentence);
/**
 * Check to see if we are given a simple literal (atom or a negation of an
 * atom).
 */
bool isSimpleLiteral(const Sentence& lit);

// IMPLEMENTATION
inline Sentence::~Sentence() {};

inline Sentence* Sentence::clone() const { return doClone(); };
inline bool Sentence::operator==(const Sentence& b) const {return doEquals(b);};
inline bool Sentence::operator!=(const Sentence& b) const {return !(*this == b);};
inline std::string Sentence::toString() const {
    std::stringstream str;
    doToString(str);
    return str.str();
};
inline int Sentence::precedence() const { return doPrecedence(); };
inline bool Sentence::contains(const Sentence& s) const { return doContains(s);};

inline Sentence* new_clone(const Sentence& t) {
    return t.clone();
};

inline std::size_t hash_value(const Sentence& s) {
    return s.doHashValue();
}

inline SISet Sentence::dSatisfied(const Model& m, const Domain& d) const {
    SISet set = satisfied(m, d, false);
    set.makeDisjoint();
    return set;
}

inline SISet Sentence::dSatisfied(const Model& m, const Domain& d, const SISet &where) const {
    SISet set = satisfied(m, d, false);
    set = intersection(set, where);
    set.makeDisjoint();
    return set;
}

inline SISet Sentence::dNotSatisfied(const Model& m, const Domain& d) const {
    SISet set = satisfied(m, d, false);
    set = set.compliment();
    set.makeDisjoint();
    return set;
}

inline SISet Sentence::dNotSatisfied(const Model& m, const Domain& d, const SISet& where) const {
    SISet set = satisfied(m, d, false);
    set = intersection(set.compliment(), where);
    set.makeDisjoint();
    return set;
}

inline TQConstraints::TQConstraints()
    : mustBeIn(false, Interval(0,0)), mustNotBeIn(false, Interval(0,0)) {}

inline TQConstraints::TQConstraints(const Interval& maxInterval)
    : mustBeIn(false, maxInterval), mustNotBeIn(false, maxInterval) {}

inline bool TQConstraints::operator==(const TQConstraints& b) const {
    return (mustBeIn == b.mustBeIn && mustNotBeIn == b.mustNotBeIn);
}
inline bool TQConstraints::operator!=(const TQConstraints& b) const {
    return !this->operator ==(b);
}

inline bool TQConstraints::empty() const {
    return (mustBeIn.size() == 0) && (mustNotBeIn.size() == 0);
}

template <class Archive>
void TQConstraints::serialize(Archive& ar, const unsigned int version) {
    ar & mustBeIn;
    ar & mustNotBeIn;
}
#endif  // SENTENCE_H
