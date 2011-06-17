/*
 * wsentence.h
 *
 *  Created on: May 21, 2011
 *      Author: joe
 */


#ifndef WSENTENCE_H_
#define WSENTENCE_H_

#include <boost/shared_ptr.hpp>
#include "sentence.h"

class WSentence {
public:
	WSentence(const boost::shared_ptr<Sentence>& s, unsigned int w) : s_(s), w_(w) {};
	virtual ~WSentence() {};

	bool operator==(const WSentence& b) const {return (*s_ == *b.s_ && w_ == b.w_);};
	const boost::shared_ptr<Sentence>& sentence() { return s_;}
	boost::shared_ptr<const Sentence> sentence() const {return s_;}
	unsigned int weight() const {return w_;};

	void setSentence(const boost::shared_ptr<Sentence>& s) {s_ = s;};
	void setWeight(unsigned int w) {w_ = w;};

	boost::shared_ptr<Sentence> s_;
	unsigned int w_;

};

#endif /* WSENTENCE_H_ */
