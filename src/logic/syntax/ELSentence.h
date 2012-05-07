/*
 * ELSentence.h
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
#include "../../SISet.h"
#include "Sentence.h"

class Model;
class Domain;

class ELSentence {
public:

    ELSentence(const boost::shared_ptr<Sentence>& s);
    ELSentence(const boost::shared_ptr<Sentence>& s, double w);
    ELSentence(const boost::shared_ptr<Sentence>& s, double w, const SISet& q);
    ELSentence(const ELSentence& s);
    virtual ~ELSentence();

    friend bool operator==(const ELSentence& a, const ELSentence& b);
    friend bool operator!=(const ELSentence& a, const ELSentence& b);

    friend std::ostream& operator<<(std::ostream& out, const ELSentence& e);

    ELSentence& operator=(ELSentence e);

    boost::shared_ptr<Sentence> sentence();
    boost::shared_ptr<const Sentence> sentence() const;
    double weight() const;
    SISet quantification() const;

    bool hasInfWeight() const;
    bool isQuantified() const;

    void setSentence(const boost::shared_ptr<Sentence>& s);
    void setWeight(double w);
    void setQuantification(const SISet& s);
    void removeQuantification();
    void setHasInfWeight(bool b);
    //void setIsQuantified(bool b);

    std::string toString() const;

    bool fullySatisfied(const Model& m, const Domain& d) const;
    SISet dSatisfied(const Model& m, const Domain& d) const;
    SISet dNotSatisfied(const Model& m, const Domain& d) const;

    friend void swap(ELSentence& a, ELSentence& b);
private:
    boost::shared_ptr<Sentence> s_;
    double w_;
    bool hasInfWeight_;
   // bool isQuantified_;
    SISet* quantification_;
};

struct IsHardClausePred : public std::unary_function<ELSentence, bool> {
    bool operator()(const ELSentence& s) const { return s.hasInfWeight(); }
};



//typedef std::vector<ELSentence> FormulaList;

// IMPLEMENTATION

inline ELSentence::ELSentence(const boost::shared_ptr<Sentence>& s)
    : s_(s), w_(1.0), hasInfWeight_(true), quantification_(0) {}

inline ELSentence::ELSentence(const boost::shared_ptr<Sentence>& s, double w)
    : s_(s), w_(w), hasInfWeight_(false), quantification_(0) {}

inline ELSentence::ELSentence(const boost::shared_ptr<Sentence>& s, double w, const SISet& q)
    : s_(s), w_(w), hasInfWeight_(false), quantification_(new SISet(q)) {}

inline ELSentence::ELSentence(const ELSentence& s)
    : s_(s.s_), w_(s.w_), hasInfWeight_(s.hasInfWeight_), quantification_(0) {
    if (s.quantification_ != 0) quantification_ = new SISet(*s.quantification_);

}

inline ELSentence::~ELSentence() {
    delete quantification_;
    quantification_ = 0;
}

inline std::ostream& operator<<(std::ostream& out, const ELSentence& e) {
    if (e.hasInfWeight_) out << "inf: ";
    else                 out << e.w_ << ": ";

    out << e.s_->toString() << " @ ";   // COME BACK AND REWRITE THIS

    if (e.quantification_ != 0) out << *e.quantification_;
    else                 out << "<everywhere>";
    return out;
}

inline ELSentence& ELSentence::operator=(ELSentence e) {
    swap(*this, e);
    return *this;
}


inline bool operator !=(const ELSentence& a, const ELSentence& b) {return !operator==(a,b);}

inline boost::shared_ptr<Sentence> ELSentence::sentence() { return s_;}

inline boost::shared_ptr<const Sentence> ELSentence::sentence() const {return s_;}

inline double ELSentence::weight() const {
    if (hasInfWeight_) {
        throw std::logic_error("logic error: cannot return infinite weight");
    }
    return w_;
}

inline SISet ELSentence::quantification() const {
    if (quantification_ == 0) {
        throw std::logic_error("logic error: no quantification applied; check with isQuantified() first");
    }
    return *quantification_;
}

inline void ELSentence::removeQuantification() {
    if (quantification_ == 0) return;
    delete quantification_;
    quantification_ = 0;
}

inline bool ELSentence::hasInfWeight() const {return hasInfWeight_;}
inline bool ELSentence::isQuantified() const {return (quantification_ != 0); }

inline void ELSentence::setSentence(const boost::shared_ptr<Sentence>& s) {s_ = s;};
inline void ELSentence::setWeight(double w) {
    w_ = w;
    hasInfWeight_ = false;
};
inline void ELSentence::setQuantification(const SISet& s) {
    SISet *newQuant = new SISet(s);
    if (quantification_ != 0) {
        delete quantification_;
    }
    quantification_ = newQuant;
    newQuant = 0;   // paranoid
};
inline void ELSentence::setHasInfWeight(bool b) { hasInfWeight_ = b;};
//inline void ELSentence::setIsQuantified(bool b) { isQuantified_ = b;};

inline void swap(ELSentence& a, ELSentence& b) {
    using std::swap;

    swap(a.s_, b.s_);
    swap(a.w_, b.w_);
    swap(a.hasInfWeight_, b.hasInfWeight_);
    swap(a.quantification_, b.quantification_);
}

#endif /* ELSENTENCE_H_ */
