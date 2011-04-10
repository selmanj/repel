#ifndef ATOM_H
#define ATOM_H

#include <string>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include "term.h"

class Atom {
public:
  Atom(std::string name) : pred(name) {};
  template <typename ForwardIterator>
  Atom(std::string name,
      ForwardIterator first,
      ForwardIterator last) : pred(name), terms(first, last) {};
  bool isGrounded() const;
  int arity() const {return terms.size();};
  std::string name() const {return pred;};

  Term& operator[] (boost::ptr_vector<Term>::size_type n) {return terms[n];};
  const Term& operator[] (boost::ptr_vector<Term>::size_type n) const {return terms[n];};
  void push_back(Term* t) {terms.push_back(t);};
  
private:
  std::string pred;
  boost::ptr_vector<Term> terms;

};
#endif
