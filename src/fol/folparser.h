#ifndef FOLPARSER_H
#define FOLPARSER_H

#include <vector>
#include <iterator>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include "foltoken.h"
#include "bad_parse.h"
#include "atom.h"
#include "../spaninterval.h"
#include "../interval.h"
#include "event.h"
#include "constant.h"
#include "variable.h"
#include "negation.h"
#include "conjunction.h"
#include "disjunction.h"
#include "liquidop.h"
#include "diamondop.h"

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
		e.details << "unexpectedly reached end of tokens while parsing type " << type << std::endl;
		throw e;
	}
	if (its.cur->type() != type) {
		bad_parse e;
		e.details << "expected type " << type << " but instead we have: " << its.cur->type() << std::endl;
		throw e;
	}
	its.cur++;
}

template <class ForwardIterator>
std::string consumeIdent(iters<ForwardIterator> &its) throw (bad_parse) {
	if (its.cur == its.last) {
		bad_parse e;
		e.details << "unexpectedly reached end of tokens while looking for identifier" << std::endl;
		throw e;
	}
	if (its.cur->type() != FOLParse::IDENT) {
		bad_parse e;
		e.details << "expected an identifier, instead we have type " << its.cur->type() << std::endl;
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
		e.details << "unexpectedly reached end of tokens while looking for variable" << std::endl;
		throw e;
	}
	if (its.cur->type() != FOLParse::VARIABLE) {
		bad_parse e;
		e.details << "expected a variable, instead we have type " << its.cur->type() << std::endl;
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
		e.details << "unexpectedly reached end of tokens while looking for number" << std::endl;
		throw e;
	}
	if (its.cur->type() != FOLParse::NUMBER) {
		bad_parse e;
		e.details << "expected a number, instead we have type " << its.cur->type() << std::endl;
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

template <class ForwardIterator>
boost::tuple<int, boost::shared_ptr<Sentence> > doParseWeightedFormula(iters<ForwardIterator> &its) {
	unsigned int weight = consumeNumber(its);
	consumeTokenType(FOLParse::COLON, its);
	boost::shared_ptr<Sentence> p = doParseFormula(its);
	return boost::tuple<int, boost::shared_ptr<Sentence> >(weight, p);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula(iters<ForwardIterator> &its) {
	return doParseFormula_exat(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_exat(iters<ForwardIterator> &its) {	// TODO: support exactly 1/at most 1
	return doParseFormula_quant(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_quant(iters<ForwardIterator> &its) {	// TODO: support quantification
	return doParseFormula_imp(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_imp(iters<ForwardIterator> &its) {
	boost::shared_ptr<Sentence> s = doParseFormula_or(its);
	while (peekTokenType(FOLParse::IMPLIES, its)) {
		consumeTokenType(FOLParse::IMPLIES, its);
		boost::shared_ptr<Sentence> s2 = doParseFormula_or(its);
		boost::shared_ptr<Negation> neg(new Negation(s));
		boost::shared_ptr<Sentence> dis(new Disjunction(neg, s2));
		s = dis;
	}
	return s;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_or(iters<ForwardIterator> &its) {
	boost::shared_ptr<Sentence> s = doParseFormula_and(its);
	while (peekTokenType(FOLParse::OR, its)) {
		consumeTokenType(FOLParse::OR, its);
		boost::shared_ptr<Sentence> s2 = doParseFormula_and(its);
		boost::shared_ptr<Sentence> dis(new Disjunction(s,s2));
		s = dis;
	}
	return s;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_and(iters<ForwardIterator> &its) {
	boost::shared_ptr<Sentence> s = doParseFormula_unary(its);
	while (peekTokenType(FOLParse::AND, its) || peekTokenType(FOLParse::SEMICOLON, its)) {
		std::set<Interval::INTERVAL_RELATION> rels;
		if (peekTokenType(FOLParse::SEMICOLON, its)) {
			consumeTokenType(FOLParse::SEMICOLON, its);
			rels.insert(Interval::MEETS);	// meets is the default in this case;
		} else {
			consumeTokenType(FOLParse::AND, its);
			rels = doParseRelationList(its);
			if (rels.empty()) {
				rels = Conjunction::defaultRelations();
			}
		}
		boost::shared_ptr<Sentence> s2 = doParseFormula_unary(its);
		boost::shared_ptr<Sentence> con(new Conjunction(s,s2,rels.begin(), rels.end()));
		s = con;
	}
	return s;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_unary(iters<ForwardIterator> &its) {
	if (peekTokenType(FOLParse::NOT, its)) {
		consumeTokenType(FOLParse::NOT, its);
		boost::shared_ptr<Sentence> s = doParseFormula(its);

		boost::shared_ptr<Sentence> neg(new Negation(s));
		return neg;
	} else if (peekTokenType(FOLParse::DIAMOND, its)) {
		consumeTokenType(FOLParse::DIAMOND, its);
		std::set<Interval::INTERVAL_RELATION> relations = doParseRelationList(its);
		if (relations.empty()) {
			// use default
			relations = DiamondOp::defaultRelations();
		}
		boost::shared_ptr<Sentence> s = doParseFormula(its);

		boost::shared_ptr<Sentence> dia(new DiamondOp(s, relations.begin(), relations.end()));
		return dia;
	} else {
		return doParseFormula_paren(its);
	}
}

template <class ForwardIterator>
std::set<Interval::INTERVAL_RELATION> doParseRelationList(iters<ForwardIterator> &its) {
	std::set<Interval::INTERVAL_RELATION> relations;
	if (peekTokenType(FOLParse::OPEN_BRACE, its)) {
		consumeTokenType(FOLParse::OPEN_BRACE, its);
		relations.insert(doParseRelation(its));
		while (peekTokenType(FOLParse::COMMA, its)) {
			consumeTokenType(FOLParse::COMMA, its);
			relations.insert(doParseRelation(its));
		}
		consumeTokenType(FOLParse::CLOSE_BRACE, its);
	}
	return relations;
}

template <class ForwardIterator>
Interval::INTERVAL_RELATION doParseRelation(iters<ForwardIterator>& its) {
	if (peekTokenType(FOLParse::EQUALS, its)) {
		consumeTokenType(FOLParse::EQUALS, its);
		return Interval::EQUALS;
	} else if (peekTokenType(FOLParse::GT, its)) {
		consumeTokenType(FOLParse::GT, its);
		return Interval::GREATERTHAN;
	} else if (peekTokenType(FOLParse::LT, its)) {
		consumeTokenType(FOLParse::LT, its);
		return Interval::LESSTHAN;
	} else {
		// looking for an ident here that matches
		std::string str = consumeIdent(its);
		if (str == "m") {
			return Interval::MEETS;
		} else if (str == "mi") {
			return Interval::MEETSI;
		} else if (str == "s") {
			return Interval::STARTS;
		} else if (str == "si") {
			return Interval::STARTSI;
		} else if (str == "d") {
			return Interval::DURING;
		} else if (str == "di") {
			return Interval::DURINGI;
		} else if (str == "f") {
			return Interval::FINISHES;
		} else if (str == "fi") {
			return Interval::FINISHESI;
		} else if (str == "o") {
			return Interval::OVERLAPS;
		} else if (str == "oi") {
			return Interval::OVERLAPSI;
		} else {
			// no more matches!
			bad_parse p;
			p.details << "no interval relation matches \"" << str << "\"" << std::endl;
			throw p;
		}
	}
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_paren(iters<ForwardIterator> &its) {
	if (peekTokenType(FOLParse::OPEN_BRACKET, its)) {
		consumeTokenType(FOLParse::OPEN_BRACKET, its);
		boost::shared_ptr<Sentence> s = doParseStaticFormula(its);
		consumeTokenType(FOLParse::CLOSE_BRACKET, its);

		boost::shared_ptr<Sentence> liq(new LiquidOp(s));
		return liq;
	} else if (peekTokenType(FOLParse::OPEN_PAREN, its)) {
		return doParseFormula(its);
	} else {
		return doParseAtom(its);
	}
}

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
	boost::shared_ptr<Sentence> s1 = doParseStaticFormula_or(its);
	while (peekTokenType(FOLParse::IMPLIES, its)) {	// Implication is just special case of disjunction, ie X -> Y = !X v Y
		consumeTokenType(FOLParse::IMPLIES, its);
		boost::shared_ptr<Sentence> s2 = doParseStaticFormula_or(its);
		boost::shared_ptr<Negation> neg(new Negation(s1));
		boost::shared_ptr<Sentence> dis(new Disjunction(neg, s2));

		s1 = dis;
	}
	return s1;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_or(iters<ForwardIterator> &its) {
	boost::shared_ptr<Sentence> s1 = doParseStaticFormula_and(its);
	while (peekTokenType(FOLParse::OR, its)) {
		consumeTokenType(FOLParse::OR, its);
		boost::shared_ptr<Sentence> s2 = doParseStaticFormula_and(its);
		boost::shared_ptr<Sentence> dis(new Disjunction(s1, s2));

		s1 = dis;
	}
	return s1;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_and(iters<ForwardIterator> &its) {
	boost::shared_ptr<Sentence> s1 = doParseStaticFormula_unary(its);
	while (peekTokenType(FOLParse::AND, its)) {
			consumeTokenType(FOLParse::AND, its);
			boost::shared_ptr<Sentence> s2 = doParseStaticFormula_unary(its);
			boost::shared_ptr<Sentence> con(new Conjunction(s1, s2));

			s1 = con;
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

template <class ForwardIterator>
boost::tuple<unsigned int, boost::shared_ptr<Sentence> > parseWeightedFormula(const ForwardIterator &first,
		const ForwardIterator &last) {
	iters<ForwardIterator> its(first, last);
	return doParseWeightedFormula(its);
}
};
#endif
