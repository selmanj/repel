/*
 * MaxWalkSat.h
 *
 *  Created on: Nov 10, 2011
 *      Author: selman.joe@gmail.com
 */

#ifndef MAXWALKSAT_H_
#define MAXWALKSAT_H_

#include <set>
#include <map>
#include <vector>
#include <iostream>

class Move;
class Model;
class Domain;
class Atom;
class ELSentence;
struct AtomStringCompare;

Model maxWalkSat(Domain& d, int numIterations, double probOfRandomMove, const Model* initialModel=0, std::ostream* dataout=0);

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



#endif /* MAXWALKSAT_H_ */
