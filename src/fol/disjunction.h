#ifndef DISJUNCTION_H
#define CONJUNCTION_H

#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "sentence.h"

class Disjunction : public Sentence {
public:
  typedef boost::ptr_vector<Sentence>::iterator iterator;
  typedef boost::ptr_vector<Sentence>::const_iterator const_iterator;

  Disjunction(const Disjunction& a) : sentences_(a.sentences_) {};
  virtual ~Disjunction() {};

  iterator begin() {return sentences_.begin();};
  const_iterator begin() const {return sentences_.begin();};
  iterator end() {return sentences_.end();};
  const_iterator end() const {return sentences_.end();};

  void push_back(Sentence* s) {sentences_.push_back(s);}
  

private:
  boost::ptr_vector<Sentence> sentences_;

  virtual Sentence* doClone() { return new Disjunction(*this); }
  virtual bool doEquals(const Sentence& s) {
    const Disjunction *con = dynamic_cast<const Disjunction*>(&s);
    if (con == NULL) {
      return false;
    }
    return sentences_ == con.sentences_; 
  }
};

#endif
