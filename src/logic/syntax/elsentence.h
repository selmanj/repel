/*
 * elsentence.h
 *
 *  Created on: May 21, 2011
 *      Author: joe
 */


#ifndef ELSENTENCE_H_
#define ELSENTENCE_H_

#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <string>
#include <sstream>
#include "siset.h"
#include "sentence.h"

class ELSentence {
public:
    ELSentence(const boost::shared_ptr<Sentence>& s);
    ELSentence(const boost::shared_ptr<Sentence>& s, unsigned int w);
    ELSentence(const boost::shared_ptr<Sentence>& s, unsigned int w, const SISet& q);
    virtual ~ELSentence();

    friend bool operator==(const ELSentence& a, const ELSentence& b);
    friend bool operator!=(const ELSentence& a, const ELSentence& b);

    boost::shared_ptr<Sentence> sentence();
    const boost::shared_ptr<const Sentence> sentence() const;
    unsigned int weight() const;
    const SISet quantification() const;

    bool hasInfWeight() const;
    bool isQuantified() const;

    void setSentence(const boost::shared_ptr<Sentence>& s);
    void setWeight(unsigned int w);
    void setQuantification(const SISet& s);
    void setHasInfWeight(bool b);
    void setIsQuantified(bool b);

    std::string toString() const;
private:
    boost::shared_ptr<Sentence> s_;
    unsigned int w_;
    bool hasInfWeight_;
    bool isQuantified_;
    SISet quantification_;

};



typedef std::vector<ELSentence> FormulaList;

// IMPLEMENTATION

inline ELSentence::ELSentence(const boost::shared_ptr<Sentence>& s)
    : s_(s), w_(1), hasInfWeight_(true), isQuantified_(false), quantification_() {}

inline ELSentence::ELSentence(const boost::shared_ptr<Sentence>& s, unsigned int w)
    : s_(s), w_(w), hasInfWeight_(false), isQuantified_(false), quantification_() {}

inline ELSentence::ELSentence(const boost::shared_ptr<Sentence>& s, unsigned int w, const SISet& q)
    : s_(s), w_(w), hasInfWeight_(false), isQuantified_(true), quantification_(q) {}

inline ELSentence::~ELSentence() {}

inline bool operator !=(const ELSentence& a, const ELSentence& b) {return operator==(a,b);}

inline boost::shared_ptr<Sentence> ELSentence::sentence() { return s_;}

inline const boost::shared_ptr<const Sentence> ELSentence::sentence() const {return s_;}

inline unsigned int ELSentence::weight() const {
    if (hasInfWeight_) {
        throw std::logic_error("logic error: cannot return infinite weight");
    }
    return w_;
}

inline const SISet ELSentence::quantification() const {
    if (!isQuantified_) {
        throw std::logic_error("logic error: no quantification applied; check with isQuantified() first");
    }
    return quantification_;
}
inline bool ELSentence::hasInfWeight() const {return hasInfWeight_;}
inline bool ELSentence::isQuantified() const {return isQuantified_; }

inline void ELSentence::setSentence(const boost::shared_ptr<Sentence>& s) {s_ = s;};
inline void ELSentence::setWeight(unsigned int w) {w_ = w; hasInfWeight_ = false;};
inline void ELSentence::setQuantification(const SISet& s) {quantification_ = s; isQuantified_ = true;};
inline void ELSentence::setHasInfWeight(bool b) { hasInfWeight_ = b;};
inline void ELSentence::setIsQuantified(bool b) { isQuantified_ = b;};


#endif /* ELSENTENCE_H_ */
