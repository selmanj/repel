#ifndef ATOM_H
#define ATOM_H

#include <string>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include "term.h"
#include "sentence.h"

class Atom : public Sentence {
public:
  Atom(std::string name) : pred(name) {};
  Atom(const Atom& a) : pred(a.pred), terms(a.terms) {};
  template <typename ForwardIterator>
  Atom(std::string name,
      ForwardIterator first,
      ForwardIterator last) : pred(name), terms(first, last) {};
  bool isGrounded() const;
  int arity() const {return terms.size();};
  std::string name() const {return pred;};

  Term& operator[] (boost::ptr_vector<Term>::size_type n) {return terms[n];};
  const Term& operator[] (boost::ptr_vector<Term>::size_type n) const {return terms[n];};
  // TODO make the at() function throw an exception
  Term& at(boost::ptr_vector<Term>::size_type n) {return terms[n];};
  const Term& at(boost::ptr_vector<Term>::size_type n) const {return terms[n];};
  void push_back(Term* t) {terms.push_back(t);};
  
private:
  std::string pred;
  boost::ptr_vector<Term> terms;

  virtual Sentence* doClone() const {return new Atom(*this);}
  virtual bool doEquals(const Sentence& t) const {
    const Atom *at = dynamic_cast<const Atom*>(&t);
    if (at == NULL) {
      return false;
    }
    return (pred == at->pred) && (terms == at->terms);
  }

};
#endif
