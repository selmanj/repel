/*
 * mcsat.h
 *
 *  Created on: Feb 7, 2012
 *      Author: selman.joe@gmail.com
 */

#ifndef MCSAT_H_
#define MCSAT_H_

class MCSat {
public:
    MCSat();

    void run(const Domain& d);

    unsigned int numIterations() const;
    Model performIteration(const Model& m, const Domain& d) const;
    void setNumIterations(unsigned int num);
private:
    unsigned int numIterations_;
};

// IMPLEMENTATION
inline MCSat::MCSat() : numIterations_(1000) {};
inline unsigned int MCSat::numIterations() const {return numIterations_;}
inline void MCSat::setNumIterations(unsigned int num) {numIterations_ = num;}


#endif /* MCSAT_H_ */
