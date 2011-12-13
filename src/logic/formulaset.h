/*
 * FormulaSet.h
 *
 *  Created on: Oct 13, 2011
 *      Author: joe
 */

#ifndef FORMULASET_H_
#define FORMULASET_H_
#include <vector>
#include <iterator>
#include "el_syntax.h"

// class FormulaSetIterator;

class FormulaSet {
public:
	typedef std::vector<ELSentence>::const_iterator const_iterator;

	FormulaSet();
	virtual ~FormulaSet();

	const_iterator begin() const {return formulas_.begin(); }
	const_iterator end() const {return formulas_.end(); }

	const std::vector<ELSentence>& formulas() const {return formulas_;}

	int size() const {return formulas_.size();}
	void addFormula(const ELSentence& sentence) {formulas_.push_back(sentence);}

private:
	std::vector<ELSentence> formulas_;
};
/*
class FormulaSetIterator : public std::iterator<std::forward_iterator_tag, WSentence> {
public:
	FormulaSetIterator() : primaryFormulas_(0), secondaryFormulas_(0), curIt_(), onPrimary_(false) {};
	FormulaSetIterator(const std::vector<WSentence>* primaryFormulas, const std::vector<WSentence>* secondaryFormulas)
	    : primaryFormulas_(primaryFormulas), secondaryFormulas_(secondaryFormulas), curIt_(), onPrimary_(true) {
		if (primaryFormulas_->begin() != primaryFormulas_->end()) {
			curIt_= primaryFormulas_->begin();
		} else if (secondaryFormulas_->begin() != secondaryFormulas_->end()) {
			onPrimary_ = false;
			curIt_ = secondaryFormulas_->begin();
		} else {
			// set to null
			primaryFormulas_ = 0;
			secondaryFormulas_ = 0;
			onPrimary_ = false;
		}
	}

	WSentence operator*() const { return *curIt_; }
	const WSentence* operator->() const { return curIt_.operator->(); }
	FormulaSetIterator& operator++() {
		curIt_++;
		if (onPrimary_ && curIt_ == primaryFormulas_->end()) {
			onPrimary_ = false;
			curIt_ = secondaryFormulas_->begin();
		}
		if (curIt_ == secondaryFormulas_->end()) {
			// set to null
			primaryFormulas_ = 0;
			secondaryFormulas_ = 0;
			onPrimary_ = false;
			curIt_ = std::vector<WSentence>::iterator();
		}
		return *this;
	}

	FormulaSetIterator operator++(int) {
		FormulaSetIterator copy(*this);
		this->operator++();
		return copy;
	}

	bool operator==(const FormulaSetIterator& b) const {
		return (curIt_ == b.curIt_
				&& primaryFormulas_ == b.primaryFormulas_
				&& secondaryFormulas_ == b.secondaryFormulas_);
	}

	bool operator!=(const FormulaSetIterator& b) const {
		return !this->operator==(b);
	}
private:
	bool onPrimary_;
	const std::vector<WSentence>* primaryFormulas_;
	const std::vector<WSentence>* secondaryFormulas_;
	std::vector<WSentence>::const_iterator curIt_;
};
*/
#endif /* FORMULASET_H_ */
