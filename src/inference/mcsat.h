/*
 * mcsat.h
 *
 *  Created on: Feb 7, 2012
 *      Author: selman.joe@gmail.com
 */

#ifndef MCSAT_H_
#define MCSAT_H_

#include <vector>
#include "../logic/domain.h"

class Model;
class MCSatSampleStrategy;

class MCSat {
public:
    static const unsigned int defNumSamples;
    static const unsigned int defWalksatIterations;
    static const double defWalksatRandomMoveProb;
    static const unsigned int defWalksatNumRandomRestarts;

    boost::unordered_set<Model> sampleSat(const Model& initialModel, const Domain& d);

    typedef std::vector<Model>::const_iterator const_iterator;

    MCSat(const Domain *d=0);
    MCSat(const MCSat& m);
    ~MCSat();

    MCSat& operator=(MCSat s);
    friend void swap(MCSat& l, MCSat& r);

    const Domain* domain() const;
    unsigned int numSamples() const;
    unsigned int walksatIterations() const;
    double walksatRandomMoveProb() const;
    unsigned int walksatNumRandomRestarts() const;
    const_iterator begin() const;
    const_iterator end() const;
    const MCSatSampleStrategy* sampleStrategy() const;
    std::size_t size() const;

    void setDomain(const Domain *d);
    void setNumSamples(unsigned int numSamples);
    void setWalksatIterations(unsigned int walksatIterations);
    void setWalksatRandomMoveProb(double walksatRandomMoveProb);
    void setWalksatNumRandomRestarts(unsigned int walksatNumRandomRestarts);
    void setSampleStrategy(MCSatSampleStrategy *strategy);
    void clear();

    void run();
private:
    static Domain applyUP(const Domain& d);   // TODO: move this to unit_prop.h eventually

    const Domain* d_;

    unsigned int numSamples_;                   // TODO: refactor these into an options attribute
    unsigned int walksatIterations_;
    double walksatRandomMoveProb_;
    unsigned int walksatNumRandomRestarts_;

    std::vector<Model> samples_;
    MCSatSampleStrategy *sampleStrategy_;
};

/**
 * Abstract Class representing the strategy used for sampling constraints.
 */
class MCSatSampleStrategy : public boost::noncopyable {
public:
    virtual ~MCSatSampleStrategy() {}

    virtual MCSatSampleStrategy* clone() const = 0;
    virtual void sampleSentences(const Model& m, const Domain& d, std::vector<ELSentence>& sampled) = 0;
};

/**
 * Helper hash/pred structs for storing ptrs to sentences in a hashmap
 */
struct sentence_ptr_hash : std::unary_function<const Sentence*, std::size_t> {
    std::size_t operator()(const Sentence* s) const {
        using boost::hash_value;
        return (s == 0 ? 0 : hash_value(*s));
    }
};

struct sentence_ptr_pred : std::binary_function<const Sentence*, const Sentence*, bool> {
    bool operator()(const Sentence* l, const Sentence* r) const {
        if (l == r) return true;
        if (l == 0 || r == 0) return false;
        return *l == *r;
    }
};

/**
 * Implementation of MCSatSampleStrategy, samples constraints based on the
 * segments that are true for all the underlying atoms in a sentence
 */
class MCSatSampleSegmentsStrategy : public MCSatSampleStrategy {
public:
    MCSatSampleSegmentsStrategy();
    MCSatSampleSegmentsStrategy(const Domain& d);
    MCSatSampleSegmentsStrategy(const MCSatSampleSegmentsStrategy& s);
    virtual ~MCSatSampleSegmentsStrategy();

    friend void swap(MCSatSampleSegmentsStrategy& l, MCSatSampleSegmentsStrategy& r);
    MCSatSampleSegmentsStrategy& operator=(MCSatSampleSegmentsStrategy other);
    virtual MCSatSampleStrategy* clone() const;

    virtual void sampleSentences(const Model& m, const Domain& d, std::vector<ELSentence>& sampled);
private:
    boost::unordered_map<Sentence*, boost::unordered_set<SpanInterval>,
    sentence_ptr_hash, sentence_ptr_pred> formulaToSegment_;
};

class MCSatSamplePerfectlyStrategy : public MCSatSampleStrategy {
    virtual MCSatSamplePerfectlyStrategy* clone() const;

    virtual void sampleSentences(const Model& m, const Domain& d, std::vector<ELSentence>& sampled);
};


// IMPLEMENTATION BELOW:
inline MCSat::MCSat(const Domain *d)
    : d_(d),
      numSamples_(defNumSamples),
      walksatIterations_(defWalksatIterations),
      walksatRandomMoveProb_(defWalksatRandomMoveProb),
      walksatNumRandomRestarts_(defWalksatNumRandomRestarts),
      samples_(),
      sampleStrategy_(0) {
    // use default strategy of segment strategy
    sampleStrategy_ = (d == 0 ? new MCSatSampleSegmentsStrategy()
                        : new MCSatSampleSegmentsStrategy(*d));
}

inline MCSat::MCSat(const MCSat& m)
    : d_(m.d_),
      numSamples_(m.numSamples_),
      walksatIterations_(m.walksatIterations_),
      walksatRandomMoveProb_(m.walksatRandomMoveProb_),
      walksatNumRandomRestarts_(m.walksatNumRandomRestarts_),
      samples_(m.samples_),
      sampleStrategy_(m.sampleStrategy_ == 0 ? 0 : m.sampleStrategy_->clone()) {}

inline MCSat::~MCSat() {
    delete sampleStrategy_;
    sampleStrategy_ = 0;
}

inline void swap(MCSat& l, MCSat& r) {
    using std::swap;

    swap(l.d_, r.d_);
    swap(l.numSamples_, r.numSamples_);
    swap(l.walksatIterations_, r.walksatIterations_);
    swap(l.walksatRandomMoveProb_, r.walksatRandomMoveProb_);
    swap(l.walksatNumRandomRestarts_, r.walksatNumRandomRestarts_);
    swap(l.samples_, r.samples_);
    swap(l.sampleStrategy_, r.sampleStrategy_);
}

inline MCSat& MCSat::operator=(MCSat s) {
    swap(s, *this);
    return *this;
}

inline const Domain* MCSat::domain() const { return d_;}
inline unsigned int MCSat::numSamples() const { return numSamples_;}
inline unsigned int MCSat::walksatIterations() const { return walksatIterations_;}
inline MCSat::const_iterator MCSat::begin() const { return samples_.begin();}
inline MCSat::const_iterator MCSat::end() const { return samples_.end();}
inline const MCSatSampleStrategy* MCSat::sampleStrategy() const { return sampleStrategy_;}
inline std::size_t MCSat::size() const {return samples_.size(); }


inline void MCSat::setDomain(const Domain* d) {d_ = d;}
inline void MCSat::setNumSamples(unsigned int numSamples) {numSamples_ = numSamples;}
inline void MCSat::setWalksatIterations(unsigned int walksatIterations) {walksatIterations_ = walksatIterations;}
inline void MCSat::setWalksatRandomMoveProb(double walksatRandomMoveProb) {walksatRandomMoveProb_ = walksatRandomMoveProb;}
inline void MCSat::setWalksatNumRandomRestarts(unsigned int walksatNumRandomRestarts) {walksatNumRandomRestarts_ = walksatNumRandomRestarts;}
inline void MCSat::setSampleStrategy(MCSatSampleStrategy *strategy) {
    sampleStrategy_ = strategy;
}

inline void MCSat::clear() {samples_.clear();}

inline MCSatSampleSegmentsStrategy::MCSatSampleSegmentsStrategy()
    : formulaToSegment_() {}


inline MCSatSampleStrategy* MCSatSampleSegmentsStrategy::clone() const {
    return new MCSatSampleSegmentsStrategy(*this);   // no state?
}

inline void swap(MCSatSampleSegmentsStrategy& l, MCSatSampleSegmentsStrategy& r) {
    using std::swap;
    swap(l.formulaToSegment_, r.formulaToSegment_);
}

inline MCSatSampleSegmentsStrategy& MCSatSampleSegmentsStrategy::operator=(MCSatSampleSegmentsStrategy other) {
    swap(*this, other);
    return *this;
}

inline MCSatSamplePerfectlyStrategy* MCSatSamplePerfectlyStrategy::clone() const {
    return new MCSatSamplePerfectlyStrategy;    //no state
}




#endif /* MCSAT_H_ */
