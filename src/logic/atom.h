#ifndef ATOM_H
#define ATOM_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include "term.h"
#include "sentence.h"
#include "sentencevisitor.h"
#include "constant.h"

class Atom : public Sentence {
public:
	typedef boost::ptr_vector<Term>::size_type size_type;

	Atom(std::string name)
	  : pred(name), terms() {};
	template <class AutoPtrIterator>
	Atom(std::string name, AutoPtrIterator first, AutoPtrIterator last)
	  : pred(name), terms(first, last) {};
	Atom(std::string name, std::auto_ptr<Term> ptr)
	  : pred(name), terms() { terms.push_back(ptr); }
	Atom(const Atom& a)
	  : pred(a.pred), terms(a.terms) {};	// shallow copy

	bool isGrounded() const;

	int arity() const {return terms.size();};
	std::string name() const {return pred;};
	std::string& name() {return pred;};

	Atom& operator=(const Atom& b);
	//Term& operator[] (boost::ptr_vector<Term>::size_type n) {return terms[n];};
	//const Term& operator[] (boost::ptr_vector<Term>::size_type n) const {return terms[n];};
	// TODO make the at() function throw an exception
	Term& at(size_type n) {return terms[n];};
	const Term& at(size_type n) const {return terms[n];};

	void push_back(std::auto_ptr<Term> t)  {terms.push_back(t);};
	virtual void visit(SentenceVisitor& v) const {
		v.accept(*this);
	}
private:
	std::string pred;
	boost::ptr_vector<Term> terms;
	//std::vector<boost::shared_ptr<Term> > terms;

	virtual Sentence* doClone() const {return new Atom(*this);};

	virtual bool doEquals(const Sentence& t) const;

	virtual void doToString(std::stringstream& str) const;

	virtual int doPrecedence() const {return 0;};
};

struct atomcmp {
	bool operator()(const Atom& a, const Atom& b) const {
		return a.toString() < b.toString();
	}
};

#endif
