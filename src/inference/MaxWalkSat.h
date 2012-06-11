/*
 * MaxWalkSat.h
 *
 *  Created on: Nov 10, 2011
 *      Author: selman.joe@gmail.com
 */

#ifndef MAXWALKSAT_H_
#define MAXWALKSAT_H_

#include <boost/random.hpp>
#include <set>
#include <map>
#include <vector>
#include <iostream>

//class Move;
class Model;
class Domain;
//class Atom;
//class ELSentence;
//struct AtomStringCompare;

/**
 * Max-Walk-Sat algorith implemented for PEL.  Finds the MAP solution (or
 * highest-scoring Model) that satisfies the given Domain's formulas.
 */
class MWSSolver {
public:
    static const unsigned int defNumIterations;
    static const double defProbOfRandomMove;

    /**
     * Construct a MWSSolver with the default settings (and no domain).
     */
    MWSSolver();

    /**
     * Construct a MWSSolver with default settings for the given domain.
     *
     * @param d     the domain the solver should work on
     */
    explicit MWSSolver(Domain* d);

    /**
     * Construct a MWSSolver with the provided settings for the given domain.
     *
     * @param numIterations     how many iterations of maxwalksat to perform
     * @param probOfRandomMove  a double ranging from 0.0 to 1.0 that specifies
     *   the probability that a random move is taken rather than the highest
     *   scoring
     * @param d                 the domain the solver should work on
     */
    MWSSolver(unsigned int numIterations, double probOfRandomMove, Domain* d);

    /**
     * Get the number of iterations the solver will run.
     *
     * @return  the number of iterations the solver will run
     */
    unsigned int numIterations() const;

    /**
     * Get the probability of a random move.
     *
     * @return  the probability a random move is taken rather than the highest
     *   scoring (value ranges from 0.0 to 1.0 inclusive).
     */
    double probOfRandomMove() const;

    /**
     * Get a pointer to the current Domain that the solver will run on.
     *
     * @return  a ptr to the current domain (possibly NULL) that the solver
     *   will run on.
     */
    Domain* domain() const;

    /**
     * Set the number of iterations that the solver should run for when
     * calling run().
     *
     * @param numIterations the number of iterations to run.
     */
    void setNumIterations(unsigned int numIterations);

    /**
     * Set the probability that a random move is made rather than the best
     * looking one.
     *
     * @param probOfRandomMove a probability ranging from 0.0 to 1.0 inclusive
     *   that specifies the probability a random move is taken.
     */
    void setProbOfRandomMove(double probOfRandomMove);

    /**
     * Set the domain to run the solver on.
     *
     * @param d a ptr to a domain (and possibly NULL) to run the solver on.
     */
    void setDomain(Domain* d);

    /**
     * Run the MaxWalkSat algorithm with the current configuration.  Note that
     * the domain object must not be NULL (or a logic_error exception is thrown).
     *
     * The search will use the domain's defaultModel() method as the seed for
     * the search.
     *
     * @param rng  the random number generator to use.
     * @return  the highest scoring model found for the set domain.
     */
    Model run(boost::mt19937& rng);

    /**
     * Run the MaxWalkSat algorithm with the current configuration.  Note that
     * the domain object must not be NULL (or a logic_error exception is thrown).
     *
     * @param rng  the random number generator to use.
     * @param initialModel  the initial model to start the search with.
     * @return  the highest scoring model found for the set domain.
     */
    Model run(boost::mt19937& rng, const Model& initialModel);
private:
    unsigned int numIterations_;
    double probOfRandomMove_;
    Domain* domain_;
};

// IMPLEMENTATION
inline MWSSolver::MWSSolver()
    : numIterations_(defNumIterations),
      probOfRandomMove_(defProbOfRandomMove),
      domain_(NULL) {}

inline MWSSolver::MWSSolver(Domain* d)
    : numIterations_(defNumIterations),
      probOfRandomMove_(defProbOfRandomMove),
      domain_(d) {};

inline MWSSolver::MWSSolver(unsigned int numIterations,
        double probOfRandomMove,
        Domain* d)
    : numIterations_(numIterations),
      probOfRandomMove_(probOfRandomMove),
      domain_(d) {
    if (probOfRandomMove < 0.0 || probOfRandomMove > 1.0) {
        std::logic_error e("probOfRandomMove is out of range for MWSSolver");
        throw e;
    }
}

inline unsigned int MWSSolver::numIterations() const {
    return numIterations_;
}

inline double MWSSolver::probOfRandomMove() const {
    return probOfRandomMove_;
}

inline Domain* MWSSolver::domain() const {
    return domain_;
}

inline void MWSSolver::setNumIterations(unsigned int numIterations) {
    numIterations_ = numIterations;
}

inline void MWSSolver::setProbOfRandomMove(double probOfRandomMove) {
    if (probOfRandomMove < 0.0 || probOfRandomMove > 1.0) {
        std::logic_error e("probOfRandomMove is out of range for MWSSolver");
        throw e;
    }
    probOfRandomMove_ = probOfRandomMove;
}

inline void MWSSolver::setDomain(Domain* d) {
    domain_ = d;
}


/*

Model maxWalkSat(Domain& d,
        int numIterations,
        double probOfRandomMove,
        boost::mt19937& rng,
        const Model* initialModel=0,
        std::ostream* dataout=0);

namespace {
    typedef std::set<int> FormSet;
    typedef std::map<Atom, FormSet, AtomStringCompare> AtomOccurences;

    struct score_pair {
        double totalScore;
        std::vector<double> formScores;
    };


    AtomOccurences findAtomOccurences(const std::vector<ELSentence>& sentences);
    // note, model m is assumed to have had move applied already!
    score_pair computeScoresForMove(const Domain& d,
            const Model& m,
            const Move& move,
            double currentScore,
            const std::vector<double>& curFormScores,
            const AtomOccurences& occurs);

    struct row_out {
        row_out() : out(0), first(true) {}
        row_out(std::ostream* o) : out(o), first(true) {}
        ~row_out() {if (out) *out << std::endl;}

        template <typename T>
        row_out& operator<<(T a) {
            if (!out) return *this;
            if (!first) {
                *out << ", ";
            } else {
                first = false;
            }
            *out << a;
            return *this;
        }

        std::ostream* out;
        bool first;
    };

}


*/
#endif /* MAXWALKSAT_H_ */
