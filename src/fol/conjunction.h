#ifndef CONJUNCTION_H
#define CONJUNCTION_H

#include <boost/shared_ptr.hpp>
#include "sentence.h"

class Conjunction : public Sentence {
public:
  typedef std::vector<boost::shared_ptr<Sentence> >::iterator iterator;
  typedef std::vector<boost::shared_ptr<Sentence> >::const_iterator const_iterator;

  Conjunction() {};
  Conjunction(const Conjunction& a) : sentences_(a.sentences_) {};
  virtual ~Conjunction() {};

  iterator begin() {return sentences_.begin();};
  const_iterator begin() const {return sentences_.begin();};
  iterator end() {return sentences_.end();};
  const_iterator end() const {return sentences_.end();};

  //void push_back(Sentence* s) {sentences_.push_back(s);};
  void push_back(boost::shared_ptr<Sentence> s){sentences_.push_back(s);};
  

private:
  std::vector<boost::shared_ptr<Sentence> > sentences_;

  virtual Sentence* doClone() const { return new Conjunction(*this); }
  virtual bool doEquals(const Sentence& s) const {
    const Conjunction *con = dynamic_cast<const Conjunction*>(&s);
    if (con == NULL) {
      return false;
    }
    return sentences_ == con->sentences_;
  }
};

#endif
