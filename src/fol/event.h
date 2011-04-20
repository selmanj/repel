#ifndef EVENT_H
#define EVENT_H

#include <boost/shared_ptr.hpp>
#include "../spaninterval.h"
#include "atom.h"

class Event {
public:
  Event(Atom fact, SpanInterval at) : a(new Atom(fact)), i(at) {};
  Event(boost::shared_ptr<Atom> fact, SpanInterval at) : a(fact), i(at) {};

  boost::shared_ptr<Atom> fact() const {return a;}
  SpanInterval at() const {return i;}
private:
  boost::shared_ptr<Atom> a;
  SpanInterval i;
};
#endif
