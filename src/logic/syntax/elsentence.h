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
#include "sentence.h"

class ELSentence {
public:
	ELSentence(const boost::shared_ptr<Sentence>& s)
		: s_(s), w_(1), hasInfWeight_(true) {};
	ELSentence(const boost::shared_ptr<Sentence>& s, unsigned int w)
		: s_(s), w_(w), hasInfWeight_(false) {};
	virtual ~ELSentence() {};

	bool operator==(const ELSentence& b) const {return (*s_ == *b.s_ && w_ == b.w_ && hasInfWeight_ == b.hasInfWeight_);};
	boost::shared_ptr<Sentence> sentence() { return s_;}
	const boost::shared_ptr<const Sentence> sentence() const {return s_;}
	bool hasInfWeight() const {return hasInfWeight_;}
	unsigned int weight() const {
		if (hasInfWeight_) {
			throw std::logic_error("logic error: cannot return infinite weight");
		}
		return w_;
	};

	void setSentence(const boost::shared_ptr<Sentence>& s) {s_ = s;};
	void setWeight(unsigned int w) {w_ = w; hasInfWeight_ = false;};
	void setHasInfWeight() {w_ = 1; hasInfWeight_ = true;};

private:
	boost::shared_ptr<Sentence> s_;
	unsigned int w_;
	bool hasInfWeight_;

};

#endif /* ELSENTENCE_H_ */
