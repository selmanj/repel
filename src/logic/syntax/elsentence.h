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
	ELSentence(const boost::shared_ptr<Sentence>& s)
		: s_(s), w_(1), hasInfWeight_(true), isQuantified_(false), quantification_() {};
	ELSentence(const boost::shared_ptr<Sentence>& s, unsigned int w)
		: s_(s), w_(w), hasInfWeight_(false), isQuantified_(false), quantification_() {};
	ELSentence(const boost::shared_ptr<Sentence>& s, unsigned int w, const SISet& q)
		: s_(s), w_(w), hasInfWeight_(false), isQuantified_(true), quantification_(q) {};
	virtual ~ELSentence() {};

	//bool operator==(const ELSentence& b) const {return (*s_ == *b.s_ && w_ == b.w_ && hasInfWeight_ == b.hasInfWeight_);};
	boost::shared_ptr<Sentence> sentence() { return s_;}
	const boost::shared_ptr<const Sentence> sentence() const {return s_;}

	unsigned int weight() const {
		if (hasInfWeight_) {
			throw std::logic_error("logic error: cannot return infinite weight");
		}
		return w_;
	};

	const SISet quantification() const {
		if (!isQuantified_) {
			throw std::logic_error("logic error: no quantification applied; check with isQuantified() first");
		}
		return quantification_;
	}

	bool hasInfWeight() const {return hasInfWeight_;}
	bool isQuantified() const { return isQuantified_; }

	void setSentence(const boost::shared_ptr<Sentence>& s) {s_ = s;};
	void setWeight(unsigned int w) {w_ = w; hasInfWeight_ = false;};
	void setQuantification(const SISet& s) {quantification_ = s; isQuantified_ = true;};
	void setHasInfWeight() {w_ = 1; hasInfWeight_ = true;};

	std::string toString() const {
		std::stringstream str;

		if (hasInfWeight_) {
			str << "inf: ";
		} else {
			str << w_ << ": ";
		}
		str << s_->toString();
		if (isQuantified_) {
			str << " @ " << quantification_.toString();
		}

		return str.str();
	}
private:
	boost::shared_ptr<Sentence> s_;
	unsigned int w_;
	bool hasInfWeight_;
	bool isQuantified_;
	SISet quantification_;

};

#endif /* ELSENTENCE_H_ */
