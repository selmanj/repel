#ifndef ATOM_H
#define ATOM_H

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "term.h"
#include "sentence.h"

class Atom : public Sentence {
public:
	typedef std::vector<boost::shared_ptr<Term> >::size_type size_type;

	Atom(std::string name) : pred(name) {};
	Atom(const Atom& a) : pred(a.pred), terms(a.terms) {};	// shallow copy
	template <typename ForwardIterator>
	Atom(std::string name,
			ForwardIterator first,
			ForwardIterator last) : pred(name) {
		ForwardIterator it = first;
		while (it != last) {
			boost::shared_ptr<Term> t(it->clone());
			terms.push_back(t);
			it++;
		}
	};
	bool isGrounded() const;
	int arity() const {return terms.size();};
	std::string name() const {return pred;};

	//Term& operator[] (boost::ptr_vector<Term>::size_type n) {return terms[n];};
	//const Term& operator[] (boost::ptr_vector<Term>::size_type n) const {return terms[n];};
	// TODO make the at() function throw an exception
	boost::shared_ptr<Term> at(size_type n) {return terms[n];};
	void push_back(const boost::shared_ptr<Term>& t)  {terms.push_back(t);};

private:
	std::string pred;
	//boost::ptr_vector<Term> terms;
	std::vector<boost::shared_ptr<Term> > terms;

	virtual Sentence* doClone() const {return new Atom(*this);};	// TODO is shallow copy what we want here?

	virtual bool doEquals(const Sentence& t) const {
		const Atom *at = dynamic_cast<const Atom*>(&t);
		if (at == NULL) {
			return false;
		}
		return (pred == at->pred) && (terms == at->terms);
	};

	virtual void doToString(std::string& str) const {
		str += pred;
		str += "(";
		for (std::vector<boost::shared_ptr<Term> >::const_iterator it = terms.begin();
				it != terms.end();
				it++) {
			str += (*it)->toString();
			if (it + 1 != terms.end()) {
				str += ", ";
			}
		}
		str += ")";
	};

	virtual int doPrecedence() const {
		return 0;
	};

};
#endif
