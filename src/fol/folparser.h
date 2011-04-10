#ifndef FOLPARSER_H
#define FOLPARSER_H

#include <vector>
#include <iterator>
#include <string>
#include "foltoken.h"
#include "bad_parse.h"
#include "atom.h"
#include "../spaninterval.h"
#include "event.h"
#include "constant.h"


// anonymous namespace for helper functions
namespace {
  template <class ForwardIterator>
  struct iters {
    iters(ForwardIterator c, ForwardIterator l) : cur(c), last(l) {};
    ForwardIterator cur;
    ForwardIterator last;
  };

  template <class ForwardIterator>
  bool peekTokenType(FOLParse::FOL_TOKEN_TYPE type, iters<ForwardIterator> &its) {
    if (its.cur == its.last) return false;
    return its.cur->type() == type;
  }

  template <class ForwardIterator>
  void consumeTokenType(FOLParse::FOL_TOKEN_TYPE type,
      iters<ForwardIterator> &its) throw (bad_parse) {
    if (its.cur == its.last) {
      bad_parse e;
      throw e;
    }
    if (its.cur->type() != type) {
      bad_parse e;
      throw e;
    }
    its.cur++;
  }

  template <class ForwardIterator>
  std::string consumeIdent(iters<ForwardIterator> &its) throw (bad_parse) {
    if (its.cur == its.last) {
      bad_parse e;
      throw e;
    }
    if (its.cur->type() != FOLParse::IDENT) {
      bad_parse e;
      throw e;
    }
    std::string name = its.cur->contents();
    its.cur++;
    return name;
  }

  template <class ForwardIterator>
  unsigned int consumeNumber(iters<ForwardIterator> &its) throw (bad_parse) {
    if (its.cur == its.last) {
      bad_parse e;
      throw e; 
    }
    if (its.cur->type() != FOLParse::NUMBER) {
      bad_parse e;
      throw e;
    }

    // use iostream to convert to int
    std::istringstream in(its.cur->contents());
    int num;
    in >> num;
    its.cur++;
    return num;
  }

  template <class ForwardIterator>
  Event doParseEvent(iters<ForwardIterator> &its) {
    Atom a = doParseGroundAtom(its);
    consumeTokenType(FOLParse::AT, its);
    SpanInterval i = doParseInterval(its);

    return Event(a, i);
  }

  template <class ForwardIterator>
  SpanInterval doParseInterval(iters<ForwardIterator> &its) {
    consumeTokenType(FOLParse::OPEN_BRACKET, its);
    return doParseInterval2(its);
  }

  template <class ForwardIterator>
  SpanInterval doParseInterval2(iters<ForwardIterator> &its) {
    if (peekTokenType(FOLParse::NUMBER, its)) {
      unsigned int i = consumeNumber(its);
      return doParseInterval3(i, its);
    } else {
      consumeTokenType(FOLParse::OPEN_BRACKET, its);
      unsigned int i = consumeNumber(its); 
      consumeTokenType(FOLParse::COMMA, its);
      unsigned int j = consumeNumber(its); 
      consumeTokenType(FOLParse::CLOSE_BRACKET, its);
      consumeTokenType(FOLParse::COMMA, its);
      consumeTokenType(FOLParse::OPEN_BRACKET, its);
      unsigned int k = consumeNumber(its);
      consumeTokenType(FOLParse::COMMA, its);
      unsigned int l = consumeNumber(its);
      consumeTokenType(FOLParse::CLOSE_BRACKET, its);
      consumeTokenType(FOLParse::CLOSE_BRACKET, its);

      return SpanInterval(Interval(i,j), Interval(k,l));
    }
  }

  template <class ForwardIterator>
  SpanInterval doParseInterval3(unsigned int i, iters<ForwardIterator> &its) {
    if (peekTokenType(FOLParse::COMMA, its)) {
      consumeTokenType(FOLParse::COMMA, its);
      unsigned int k = consumeNumber(its );
      consumeTokenType(FOLParse::CLOSE_BRACKET, its);
      return SpanInterval(Interval(i,i), Interval(k,k));
    } else {
      consumeTokenType(FOLParse::COLON, its);
      unsigned int k = consumeNumber(its);
      consumeTokenType(FOLParse::CLOSE_BRACKET, its);
      return SpanInterval(Interval(i,k), Interval(i,k));
    }
  }

  template <class ForwardIterator>
  Atom doParseGroundAtom(iters<ForwardIterator> &its) {
    std::string predName = consumeIdent(its);
    Atom a(predName);

    consumeTokenType(FOLParse::OPEN_PAREN, its);
    a.push_back(new Constant(consumeIdent(its))); // ownership transfered to atom
    while (peekTokenType(FOLParse::COMMA, its)) {
      consumeTokenType(FOLParse::COMMA, its);
      a.push_back(new Constant(consumeIdent(its)));
    }
    consumeTokenType(FOLParse::CLOSE_PAREN, its);
    return a;
  }
};

namespace FOLParse 
{
  template <class ForwardIterator>
  Event parseEvent(const ForwardIterator &first,
      const ForwardIterator &last) {
    iters<ForwardIterator> its(first, last);
    return doParseEvent(its);
  };

  template <class ForwardIterator>
  SpanInterval parseInterval(const ForwardIterator &first,
      const ForwardIterator &last) {
    iters<ForwardIterator> its(first, last);
    return doParseInterval(its);
  };

  template <class ForwardIterator>
  Atom parseGroundAtom(const ForwardIterator &first,
      const ForwardIterator &last) {
    iters<ForwardIterator> its(first, last);
    return doParseGroundAtom(its);
  };
};
#endif
