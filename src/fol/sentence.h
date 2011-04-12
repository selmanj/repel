#ifndef SENTENCE_H
#define SENTENCE_H

#include <boost/utility.hpp>

class Sentence : boost::noncopyable {
public:
  virtual ~Sentence() {};
  Sentence* clone() const { return doClone(); };
  bool operator==(const Sentence& b) const {return doEquals(b);};

private:
  virtual Sentence* doClone() const = 0;
  virtual bool doEquals(const Sentence& t) const = 0;
};

inline Sentence* new_clone(const Sentence& t) {
  return t.clone();
}
#endif
