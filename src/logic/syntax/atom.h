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

	Atom(std::string name);
	template <class AutoPtrIterator>
	Atom(std::string name, AutoPtrIterator first, AutoPtrIterator last);
	Atom(std::string name, std::auto_ptr<Term> ptr);
	Atom(const Atom& a);	// shallow copy

	bool isGrounded() const;

	int arity() const;
	std::string name() const;
	std::string& name();

	Atom& operator=(const Atom& b);

	Term& at(size_type n);
	const Term& at(size_type n) const;

	void push_back(std::auto_ptr<Term> t);
	virtual void visit(SentenceVisitor& v) const;
private:
	std::string pred;
	boost::ptr_vector<Term> terms;
	//std::vector<boost::shared_ptr<Term> > terms;

	virtual Sentence* doClone() const;
	virtual bool doEquals(const Sentence& t) const;
	virtual void doToString(std::stringstream& str) const;
	virtual int doPrecedence() const;
};

struct atomcmp {
	bool operator()(const Atom& a, const Atom& b) const {
		return a.toString() < b.toString();
	}
};

// IMPLEMENTATION

inline Atom::Atom(std::string name)
  : pred(name), terms() {};
template <class AutoPtrIterator>
Atom::Atom(std::string name, AutoPtrIterator first, AutoPtrIterator last)
  : pred(name), terms(first, last) {};
inline Atom::Atom(std::string name, std::auto_ptr<Term> ptr)
  : pred(name), terms() { terms.push_back(ptr); }
inline Atom::Atom(const Atom& a)
  : pred(a.pred), terms(a.terms) {};	// shallow copy

inline int Atom::arity() const {return terms.size();};
inline std::string Atom::name() const {return pred;};
inline std::string& Atom::name() {return pred;};

// TODO make the at() function throw an exception
inline Term& Atom::at(size_type n) {return terms[n];};
inline const Term& Atom::at(size_type n) const {return terms[n];};

inline void Atom::push_back(std::auto_ptr<Term> t)  {terms.push_back(t);};

inline void Atom::visit(SentenceVisitor& v) const {
	v.accept(*this);
}


inline Sentence* Atom::doClone() const {return new Atom(*this);};
inline int Atom::doPrecedence() const {return 0;};

#endif
