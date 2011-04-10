#ifndef EVENT_H
#define EVENT_H

#include "../spaninterval.h"
#include "atom.h"

class Event {
public:
  Event(Atom fact, SpanInterval at) : a(fact), i(at) {};

  Atom fact() const {return a;}
  SpanInterval at() const {return i;}
private:
  Atom a;
  SpanInterval i;
};
#endif
