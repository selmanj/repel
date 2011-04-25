#ifndef FOLPARSER_H
#define FOLPARSER_H

#include <vector>
#include <iterator>
#include <string>
#include <boost/shared_ptr.hpp>
#include "foltoken.h"
#include "bad_parse.h"
#include "atom.h"
#include "../spaninterval.h"
#include "event.h"
#include "constant.h"
#include "variable.h"
#include "negation.h"
#include "conjunction.h"
#include "disjunction.h"


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
std::string consumeVariable(iters<ForwardIterator> &its) throw (bad_parse) {
	if (its.cur == its.last) {
		bad_parse e;
		throw e;
	}
	if (its.cur->type() != FOLParse::VARIABLE) {
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
boost::shared_ptr<Event> doParseEvent(iters<ForwardIterator> &its) {
	boost::shared_ptr<Atom> a = doParseGroundAtom(its);
	consumeTokenType(FOLParse::AT, its);
	SpanInterval i = doParseInterval(its);

	boost::shared_ptr<Event> p(new Event(a,i));
	return p;
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
boost::shared_ptr<Atom> doParseGroundAtom(iters<ForwardIterator> &its) {
	std::string predName = consumeIdent(its);
	boost::shared_ptr<Atom> a(new Atom(predName));

	consumeTokenType(FOLParse::OPEN_PAREN, its);
	boost::shared_ptr<Constant> c(new Constant(consumeIdent(its)));
	a->push_back(c); // ownership transfered to atom
	while (peekTokenType(FOLParse::COMMA, its)) {
		consumeTokenType(FOLParse::COMMA, its);
		boost::shared_ptr<Constant> cnext(new Constant(consumeIdent(its)));
		a->push_back(cnext);
	}
	consumeTokenType(FOLParse::CLOSE_PAREN, its);
	return a;
}

template <class ForwardIterator>
boost::shared_ptr<Atom> doParseAtom(iters<ForwardIterator> &its) {
	std::string predName = consumeIdent(its);
	boost::shared_ptr<Atom> a(new Atom(predName));

	consumeTokenType(FOLParse::OPEN_PAREN, its);
	if (peekTokenType(FOLParse::IDENT, its)) {
		boost::shared_ptr<Constant> c(new Constant(consumeIdent(its)));
		a->push_back(c);
	} else {
		boost::shared_ptr<Variable> v(new Variable(consumeVariable(its)));
		a->push_back(v);
	}
	while (peekTokenType(FOLParse::COMMA, its)) {
		consumeTokenType(FOLParse::COMMA, its);
		if (peekTokenType(FOLParse::IDENT, its)) {
			boost::shared_ptr<Constant> c(new Constant(consumeIdent(its)));
			a->push_back(c);
		} else {
			boost::shared_ptr<Variable> v(new Variable(consumeVariable(its)));
			a->push_back(v);
		}
	}
	consumeTokenType(FOLParse::CLOSE_PAREN, its);
	return a;
}

/*
  template <class ForwardIterator>
  void doParseWeightedFormula(iters<ForwardIterator> &its) {
    unsigned int weight = consumeNumber(its);
    consumeTokenType(FOLParse::COLON, its);
    doParseFormula(its);
  }

  template <class ForwardIterator>
  void doParseFormula(iters<ForwardIterator> &its) {

  }
 */
template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula(iters<ForwardIterator> &its) {
	return doParseStaticFormula_exat(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_exat(iters<ForwardIterator> &its) {	// TODO: support exactly 1/at most 1
	return doParseStaticFormula_quant(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_quant(iters<ForwardIterator> &its) {	// TODO: support quantification
	return doParseStaticFormula_imp(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_imp(iters<ForwardIterator> &its) {	// TODO: support double-implication
	boost::shared_ptr<Sentence> s1 = doParseStaticFormula_conn(its);
	while (peekTokenType(FOLParse::IMPLIES, its)) {	// Implication is just special case of disjunction, ie X -> Y = !X v Y
		consumeTokenType(FOLParse::IMPLIES, its);
		boost::shared_ptr<Sentence> s2 = doParseStaticFormula_conn(its);
		boost::shared_ptr<Negation> neg(new Negation(s1));
		boost::shared_ptr<Disjunction> dis(new Disjunction());

		dis->push_back(neg);
		dis->push_back(s2);

		s1 = boost::static_pointer_cast<Sentence>(dis);
	}
	return s1;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_conn(iters<ForwardIterator> &its) {
	boost::shared_ptr<Sentence> s1 = doParseStaticFormula_unary(its);
	while (peekTokenType(FOLParse::AND, its) || peekTokenType(FOLParse::OR, its)) {
		if (peekTokenType(FOLParse::AND, its)) {
			consumeTokenType(FOLParse::AND, its);
			boost::shared_ptr<Sentence> s2 = doParseStaticFormula_unary(its);
			boost::shared_ptr<Conjunction> con(new Conjunction);
			con->push_back(s1);
			con->push_back(s2);
			s1 = boost::static_pointer_cast<Sentence>(con);
		} else {	// must be disjunction
			consumeTokenType(FOLParse::OR, its);
			boost::shared_ptr<Sentence> s2 = doParseStaticFormula_unary(its);
			boost::shared_ptr<Disjunction> dis(new Disjunction());
			dis->push_back(s1);
			dis->push_back(s2);
			s1 = boost::static_pointer_cast<Sentence>(dis);
		}
	}
	return s1;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_unary(iters<ForwardIterator> &its) {
	if (peekTokenType(FOLParse::NOT, its)) {
		boost::shared_ptr<Sentence> s(new Negation(doParseStaticFormula(its)));
		return s;
	} else {
		return doParseStaticFormula_paren(its);
	}
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_paren(iters<ForwardIterator> &its) {
	boost::shared_ptr<Sentence> s;
	if (peekTokenType(FOLParse::OPEN_PAREN, its)) {
		consumeTokenType(FOLParse::OPEN_PAREN, its);
		s = doParseStaticFormula(its);
		consumeTokenType(FOLParse::CLOSE_PAREN, its);
	} else {
		s = doParseAtom(its);
	}
	return s;
}
};

namespace FOLParse 
{
template <class ForwardIterator>
boost::shared_ptr<Event> parseEvent(const ForwardIterator &first,
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
boost::shared_ptr<Atom> parseGroundAtom(const ForwardIterator &first,
		const ForwardIterator &last) {
	iters<ForwardIterator> its(first, last);
	return doParseGroundAtom(its);
};
template <class ForwardIterator>
boost::shared_ptr<Atom> parseAtom(const ForwardIterator &first,
		const ForwardIterator &last) {
	iters<ForwardIterator> its(first, last);
	return doParseAtom(its);
};

template <class ForwardIterator>
boost::shared_ptr<Sentence> parseStaticFormula(const ForwardIterator &first,
		const ForwardIterator &last) {
	iters<ForwardIterator> its(first, last);
	return doParseStaticFormula(its);
}
};
#endif
