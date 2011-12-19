#ifndef FOLPARSER_H
#define FOLPARSER_H

#include <vector>
#include <iterator>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "follexer.h"
#include "foltoken.h"
#include "domain.h"
#include "bad_parse.h"
#include "../spaninterval.h"
#include "../interval.h"
#include "el_syntax.h"
#include "../log.h"

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
bool endOfTokens(iters<ForwardIterator> &its) {
	return its.cur == its.last;
}


template <class ForwardIterator>
void doParseEvents(std::vector<FOL::Event>& store, iters<ForwardIterator> &its) {
	while (!endOfTokens(its)) {
		if (peekTokenType(FOLParse::ENDL, its)) {
			consumeTokenType(FOLParse::ENDL, its);
		} else {
			std::vector<FOL::Event> events = doParseEvent(its);
			BOOST_FOREACH(FOL::Event event, events) {
				store.push_back(event);
			}
		}
	}
}

template <class ForwardIterator>
void doParseFormulas(FormulaList& store, iters<ForwardIterator> &its) {
	while (!endOfTokens(its)) {
		if (peekTokenType(FOLParse::ENDL, its)) {
			consumeTokenType(FOLParse::ENDL, its);
		} else {
			ELSentence formula = doParseWeightedFormula(its);
			store.push_back(formula);
			//store.push_back(formula);
		}
	}
}

/*
template <class ForwardIterator>
void doParseInitFormulas(FormulaSet& store, iters<ForwardIterator> &its) {
	consumeTokenType(FOLParse::INIT, its);
	while (peekTokenType(FOLParse::ENDL, its)) {
		consumeTokenType(FOLParse::ENDL, its);
	}
	consumeTokenType(FOLParse::OPEN_BRACE, its);
	while (!peekTokenType(FOLParse::CLOSE_BRACE, its)) {
		if (peekTokenType(FOLParse::ENDL, its)) {
			consumeTokenType(FOLParse::ENDL, its);
			continue;
		}
		WSentence formula = doParseWeightedFormula(its);
		store.addPrimaryFormula(formula);
	}
	consumeTokenType(FOLParse::CLOSE_BRACE, its);
}
*/

template <class ForwardIterator>
std::vector<FOL::Event> doParseEvent(iters<ForwardIterator> &its) {
	std::vector<FOL::Event> events;
	bool truthVal = true;
	if (peekTokenType(FOLParse::NOT, its)) {
		consumeTokenType(FOLParse::NOT, its);
		truthVal = false;
	}
	boost::shared_ptr<Atom> s = doParseGroundAtom(its);
	consumeTokenType(FOLParse::AT, its);
	if (peekTokenType(FOLParse::OPEN_BRACE, its)) {
		consumeTokenType(FOLParse::OPEN_BRACE, its);
		if (peekTokenType(FOLParse::CLOSE_BRACE, its)) {
			consumeTokenType(FOLParse::CLOSE_BRACE, its);
			return events;
		}
		while (!peekTokenType(FOLParse::CLOSE_BRACE, its)) {
			FOL::Event event(s, doParseInterval(its), truthVal);
			events.push_back(event);
			if (!peekTokenType(FOLParse::CLOSE_BRACE, its)) consumeTokenType(FOLParse::COMMA, its);
		}
		consumeTokenType(FOLParse::CLOSE_BRACE, its);

	} else {
		events.push_back(FOL::Event(s, doParseInterval(its), truthVal));
	}

	return events;
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
		consumeTokenType(FOLParse::OPEN_PAREN, its);
		unsigned int i = consumeNumber(its);
		consumeTokenType(FOLParse::COMMA, its);
		unsigned int j = consumeNumber(its);
		consumeTokenType(FOLParse::CLOSE_PAREN, its);
		consumeTokenType(FOLParse::COMMA, its);
		consumeTokenType(FOLParse::OPEN_PAREN, its);
		unsigned int k = consumeNumber(its);
		consumeTokenType(FOLParse::COMMA, its);
		unsigned int l = consumeNumber(its);
		consumeTokenType(FOLParse::CLOSE_PAREN, its);
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
boost::shared_ptr<Sentence> doParseEventLiteral(iters<ForwardIterator> &its) {
	if (peekTokenType(FOLParse::NOT, its)) {
		consumeTokenType(FOLParse::NOT, its);
		boost::shared_ptr<Atom> atom = doParseGroundAtom(its);
		boost::shared_ptr<Sentence> s(new Negation(atom));
		return s;
	}
	return doParseGroundAtom(its);
}

template <class ForwardIterator>
boost::shared_ptr<Atom> doParseGroundAtom(iters<ForwardIterator> &its) {
	std::string predName = consumeIdent(its);
	boost::shared_ptr<Atom> a(new Atom(predName));

	consumeTokenType(FOLParse::OPEN_PAREN, its);
	if (peekTokenType(FOLParse::CLOSE_PAREN, its)) {
		// ok, empty atom, finish parsing
		consumeTokenType(FOLParse::CLOSE_PAREN, its);
		return a;
	}
	std::auto_ptr<Term> c(new Constant(consumeIdent(its)));
	a->push_back(c); // ownership transfered to atom
	while (peekTokenType(FOLParse::COMMA, its)) {
		consumeTokenType(FOLParse::COMMA, its);
		std::auto_ptr<Term> cnext(new Constant(consumeIdent(its)));
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
		std::auto_ptr<Term> c(new Constant(consumeIdent(its)));
		a->push_back(c);
	} else if (peekTokenType(FOLParse::VARIABLE, its)){
		std::auto_ptr<Term> v(new Variable(consumeVariable(its)));
		a->push_back(v);
	} else {
		// atom with no args
		consumeTokenType(FOLParse::CLOSE_PAREN, its);
		return a;
	}
	while (peekTokenType(FOLParse::COMMA, its)) {
		consumeTokenType(FOLParse::COMMA, its);
		if (peekTokenType(FOLParse::IDENT, its)) {
			std::auto_ptr<Term> c(new Constant(consumeIdent(its)));
			a->push_back(c);
		} else {
			std::auto_ptr<Term> v(new Variable(consumeVariable(its)));
			a->push_back(v);
		}
	}
	consumeTokenType(FOLParse::CLOSE_PAREN, its);
	return a;
}

template <class ForwardIterator>
ELSentence doParseWeightedFormula(iters<ForwardIterator> &its) {
	bool hasWeight;
	unsigned int weight;
	if (peekTokenType(FOLParse::NUMBER, its)) {
		hasWeight = true;
		weight = consumeNumber(its);
	} else {
		if (peekTokenType(FOLParse::INF, its)) {
			consumeTokenType(FOLParse::INF, its);
		}
		hasWeight = false;
		weight = 0;
	}
	consumeTokenType(FOLParse::COLON, its);
	boost::shared_ptr<Sentence> p = doParseFormula(its);
	ELSentence sentence(p);
	if (hasWeight) {
		sentence.setWeight(weight);
	} else {
		sentence.setHasInfWeight(true);
	}
	// check to see if it's quantified
	if (peekTokenType(FOLParse::AT, its)) {
		consumeTokenType(FOLParse::AT, its);
		SISet set;
		if (peekTokenType(FOLParse::OPEN_BRACE, its)) {
			consumeTokenType(FOLParse::OPEN_BRACE, its);
			while (!peekTokenType(FOLParse::CLOSE_BRACE, its)) {
				SpanInterval si = doParseInterval(its);
				set.add(si);
				if (!peekTokenType(FOLParse::CLOSE_BRACE, its)) {
					consumeTokenType(FOLParse::COMMA, its);
				}
			}
			consumeTokenType(FOLParse::CLOSE_BRACE, its);
		} else {
			SpanInterval si = doParseInterval(its);
			set.add(si);
		}
		sentence.setQuantification(set);
	} else {
		sentence.setIsQuantified(false);
	}
	return sentence;
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
		boost::shared_ptr<Sentence> s = doParseFormula_unary(its);
		boost::shared_ptr<Sentence> neg(new Negation(s));
		return neg;
	} else if (peekTokenType(FOLParse::DIAMOND, its)) {
		consumeTokenType(FOLParse::DIAMOND, its);
		std::set<Interval::INTERVAL_RELATION> relations = doParseRelationList(its);
		if (relations.empty()) {
			// use default
			relations = DiamondOp::defaultRelations();
		}
		//boost::shared_ptr<Sentence> s = doParseFormula(its);
		boost::shared_ptr<Sentence> s = doParseFormula_unary(its);

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
		if (peekTokenType(FOLParse::STAR, its)) {
			// add all relations
			relations.insert(Interval::MEETS);
			relations.insert(Interval::MEETSI);
			relations.insert(Interval::OVERLAPS);
			relations.insert(Interval::OVERLAPSI);
			relations.insert(Interval::STARTS);
			relations.insert(Interval::STARTSI);
			relations.insert(Interval::DURING);
			relations.insert(Interval::DURINGI);
			relations.insert(Interval::FINISHES);
			relations.insert(Interval::FINISHESI);
			relations.insert(Interval::EQUALS);
			relations.insert(Interval::GREATERTHAN);
			relations.insert(Interval::LESSTHAN);
			consumeTokenType(FOLParse::STAR, its);
		} else {
			relations.insert(doParseRelation(its));
		}
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
		consumeTokenType(FOLParse::OPEN_PAREN, its);
		boost::shared_ptr<Sentence> s = doParseFormula(its);
		consumeTokenType(FOLParse::CLOSE_PAREN, its);
		return s;
	} else if (peekTokenType(FOLParse::TTRUE, its)) {
		consumeTokenType(FOLParse::TTRUE, its);
		boost::shared_ptr<Sentence> s(new BoolLit(true));
		return s;
	} else if (peekTokenType(FOLParse::TFALSE, its)) {
		consumeTokenType(FOLParse::TFALSE, its);
		boost::shared_ptr<Sentence> s(new BoolLit(false));
		return s;
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
		consumeTokenType(FOLParse::NOT, its);
		boost::shared_ptr<Sentence> s(new Negation(doParseStaticFormula_paren(its)));
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
	} else if (peekTokenType(FOLParse::TTRUE, its)) {
		consumeTokenType(FOLParse::TTRUE, its);
		boost::shared_ptr<Sentence> s(new BoolLit(true));
		return s;
	} else if (peekTokenType(FOLParse::TFALSE, its)) {
		consumeTokenType(FOLParse::TFALSE, its);
		boost::shared_ptr<Sentence> s(new BoolLit(false));
		return s;
	} else {
		s = doParseAtom(its);
	}
	return s;
}
};

namespace FOLParse 
{

void parseEventFile(const std::string &filename, std::vector<FOL::Event>& store) {
	std::ifstream file(filename.c_str());
	if (!file.is_open()) {
		std::runtime_error e("unable to open event file for parsing");
		throw e;
	}
	std::vector<FOLToken> tokens = FOLParse::tokenize(&file);
	//BOOST_FOREACH(FOLToken token, tokens) {
	//	std::cout << "type: " << token.type() <<  " contents: " << token.contents() << std::endl;
	//}

	iters<std::vector<FOLToken>::const_iterator > its(tokens.begin(), tokens.end());
	doParseEvents(store, its);
	file.close();
};

void parseFormulaFile(const std::string &filename, FormulaList& store) {
	std::ifstream file(filename.c_str());
	if (!file.is_open()) {
		std::runtime_error e("unable to open event file for parsing");
		throw e;
	}
	std::vector<FOLToken> tokens = FOLParse::tokenize(&file);
	iters<std::vector<FOLToken>::const_iterator> its(tokens.begin(), tokens.end());
	doParseFormulas(store, its);
	file.close();
};

template <class ForwardIterator>
void parseFormulas(const ForwardIterator &first,
		const ForwardIterator &last, FormulaList& store) {
	iters<std::vector<FOLToken>::const_iterator> its(first, last);
	doParseFormulas(store, its);
}

boost::shared_ptr<Domain> loadDomainFromFiles(const std::string &eventfile, const std::string &formulafile) {
	std::vector<FOL::Event> events;
	//std::vector<WSentence> formulas;
	FormulaList formSet;

	parseEventFile(eventfile, events);
	std::cout << "Read " << events.size() << " events from file." << std::endl;
	parseFormulaFile(formulafile, formSet);
	std::cout << "Read " << formSet.size() << " formulas from file." << std::endl;

	boost::shared_ptr<Domain> d(new Domain(events.begin(), events.end(), formSet));

	return d;
};

template <class ForwardIterator>
void parseEvents(const ForwardIterator &first,
		const ForwardIterator &last, std::vector<FOL::Event>& store) {
	iters<ForwardIterator> its(first, last);
	doParseEvents(store, its);
};

template <class ForwardIterator>
std::vector<FOL::Event> parseEvent(const ForwardIterator &first,
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
boost::shared_ptr<Sentence> parseFormula(const ForwardIterator &first,
		const ForwardIterator &last) {
	iters<ForwardIterator> its(first, last);
	return doParseFormula(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> parseStaticFormula(const ForwardIterator &first,
		const ForwardIterator &last) {
	iters<ForwardIterator> its(first, last);
	return doParseStaticFormula(its);
}

template <class ForwardIterator>
ELSentence parseWeightedFormula(const ForwardIterator &first,
		const ForwardIterator &last) {
	iters<ForwardIterator> its(first, last);
	return doParseWeightedFormula(its);
}
};
#endif
