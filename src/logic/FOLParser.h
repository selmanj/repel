#ifndef FOLPARSER_H
#define FOLPARSER_H

#include <vector>
#include <iterator>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "FOLLexer.h"
#include "FOLToken.h"
#include "Domain.h"
#include "bad_parse.h"
#include "../SpanInterval.h"
#include "../Interval.h"
#include "ELSyntax.h"
#include "../Log.h"

// anonymous namespace for helper functions
namespace {

struct find_max_interval : public std::unary_function<SpanInterval, void> {
    find_max_interval() : max() {}

    void operator()(const SpanInterval& i) {
        Interval x(i.start().start(), i.finish().finish());

        if (max.isNull()) {
            max = x;
        } else {
            max = span(max, x);
        }
    }

    Interval max;
};


template <class ForwardIterator>
struct iters {
    iters(ForwardIterator c, ForwardIterator l) : cur(c), last(l) {};
    ForwardIterator cur;
    ForwardIterator last;
};

class ParseOptions {
public:
    static const bool defAssumeClosedWorldInFacts = true;

    ParseOptions() : factsClosed_(defAssumeClosedWorldInFacts) {}

    bool assumeClosedWorldInFacts() const { return factsClosed_;}
    void setAssumeClosedWorldInFacts(bool b) {factsClosed_ = b;}

private:
    bool factsClosed_;
};

std::ostream& operator<<(std::ostream& o, const ParseOptions& p) {
    o << "{assumeClosedWorldInFacts = " << p.assumeClosedWorldInFacts() << "}";
    return o;
}

template <class ForwardIterator>
bool peekTokenType(FOLParse::TokenType type, iters<ForwardIterator> &its) {
    if (its.cur == its.last) return false;
    return its.cur->type() == type;
}

template <class ForwardIterator>
void consumeTokenType(FOLParse::TokenType type,
        iters<ForwardIterator> &its) throw (bad_parse) {
    if (its.cur == its.last) {
        bad_parse e;
        std::stringstream str;
        str << "unexpectedly reached end of tokens while parsing type " << type << std::endl;
        e.details = str.str();
        throw e;
    }
    if (its.cur->type() != type) {
        bad_parse e;
        std::stringstream str;

        str << "expected type " << type << " but instead we have: " << its.cur->type() << std::endl;
        str << "line number: " << its.cur->lineNumber() << ", column number: " << its.cur->colNumber() << std::endl;
        e.details = str.str();
        throw e;
    }
    its.cur++;
}

template <class ForwardIterator>
std::string consumeIdent(iters<ForwardIterator> &its) throw (bad_parse) {
    if (its.cur == its.last) {
        bad_parse e;
        std::stringstream str;
        str << "unexpectedly reached end of tokens while looking for identifier" << std::endl;
        e.details = str.str();
        throw e;
    }
    if (its.cur->type() != FOLParse::Identifier) {
        bad_parse e;
        std::stringstream str;
        str << "expected an identifier, instead we have type " << its.cur->type() << std::endl;
        str << "line number: " << its.cur->lineNumber() << ", column number: " << its.cur->colNumber() << std::endl;
        e.details = str.str();
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
        std::stringstream str;
        str << "unexpectedly reached end of tokens while looking for variable" << std::endl;
        e.details = str.str();
        throw e;
    }
    if (its.cur->type() != FOLParse::Variable) {
        bad_parse e;
        std::stringstream str;
        str << "expected a variable, instead we have type " << its.cur->type() << std::endl;
        str << "line number: " << its.cur->lineNumber() << ", column number: " << its.cur->colNumber() << std::endl;
        e.details = str.str();
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
        std::stringstream str;
        str << "unexpectedly reached end of tokens while looking for number" << std::endl;
        e.details = str.str();
        throw e;
    }
    if (its.cur->type() != FOLParse::Number) {
        bad_parse e;
        std::stringstream str;
        str << "expected a number, instead we have type " << its.cur->type() << std::endl;
        str << "line number: " << its.cur->lineNumber() << ", column number: " << its.cur->colNumber() << std::endl;
        e.details = str.str();
        throw e;
    }

    // use iostream to convert to int
    std::istringstream in(its.cur->contents());
    unsigned int num;
    in >> num;
    its.cur++;
    return num;
}

template <class ForwardIterator>
double consumeFloat(iters<ForwardIterator> &its) throw (bad_parse) {
    if (its.cur == its.last) {
        bad_parse e;
        std::stringstream str;
        str << "unexpectedly reached end of tokens while looking for number" << std::endl;
        e.details = str.str();
        throw e;
    }
    if (its.cur->type() != FOLParse::Float) {
        bad_parse e;
        std::stringstream str;
        str << "expected a float, instead we have type " << its.cur->type() << std::endl;
        str << "line number: " << its.cur->lineNumber() << ", column number: " << its.cur->colNumber() << std::endl;
        e.details = str.str();
        throw e;
    }

    // use iostream to convert to float
    std::istringstream in(its.cur->contents());
    double num;
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
        if (peekTokenType(FOLParse::EndLine, its)) {
            consumeTokenType(FOLParse::EndLine, its);
        } else {
            std::vector<FOL::Event> events = doParseEvent(its);
            BOOST_FOREACH(FOL::Event event, events) {
                store.push_back(event);
            }
        }
    }
}

template <class ForwardIterator>
void doParseFormulas(std::vector<ELSentence>& store, iters<ForwardIterator> &its) {
    while (!endOfTokens(its)) {
        if (peekTokenType(FOLParse::EndLine, its)) {
            consumeTokenType(FOLParse::EndLine, its);
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
    while (peekTokenType(FOLParse::EndLine, its)) {
        consumeTokenType(FOLParse::EndLine, its);
    }
    consumeTokenType(FOLParse::OpenBrace, its);
    while (!peekTokenType(FOLParse::CloseBrace, its)) {
        if (peekTokenType(FOLParse::EndLine, its)) {
            consumeTokenType(FOLParse::EndLine, its);
            continue;
        }
        WSentence formula = doParseWeightedFormula(its);
        store.addPrimaryFormula(formula);
    }
    consumeTokenType(FOLParse::CloseBrace, its);
}
*/

template <class ForwardIterator>
std::vector<FOL::Event> doParseEvent(iters<ForwardIterator> &its) {
    std::vector<FOL::Event> events;
    bool truthVal = true;
    if (peekTokenType(FOLParse::Not, its)) {
        consumeTokenType(FOLParse::Not, its);
        truthVal = false;
    }
    boost::shared_ptr<Atom> s = doParseGroundAtom(its);
    consumeTokenType(FOLParse::At, its);
    if (peekTokenType(FOLParse::OpenBrace, its)) {
        consumeTokenType(FOLParse::OpenBrace, its);
        if (peekTokenType(FOLParse::CloseBrace, its)) {
            consumeTokenType(FOLParse::CloseBrace, its);
            return events;
        }
        while (!peekTokenType(FOLParse::CloseBrace, its)) {
            FOL::Event event(s, doParseInterval(its), truthVal);
            events.push_back(event);
            if (!peekTokenType(FOLParse::CloseBrace, its)) consumeTokenType(FOLParse::Comma, its);
        }
        consumeTokenType(FOLParse::CloseBrace, its);

    } else {
        events.push_back(FOL::Event(s, doParseInterval(its), truthVal));
    }

    return events;
}

template <class ForwardIterator>
SpanInterval doParseInterval(iters<ForwardIterator> &its) {
    consumeTokenType(FOLParse::OpenBracket, its);
    return doParseInterval2(its);
}

template <class ForwardIterator>
SpanInterval doParseInterval2(iters<ForwardIterator> &its) {
    if (peekTokenType(FOLParse::Number, its)) {
        unsigned int i = consumeNumber(its);
        return doParseInterval3(i, its);
    } else {
        consumeTokenType(FOLParse::OpenParen, its);
        unsigned int i = consumeNumber(its);
        consumeTokenType(FOLParse::Comma, its);
        unsigned int j = consumeNumber(its);
        consumeTokenType(FOLParse::CloseParen, its);
        consumeTokenType(FOLParse::Comma, its);
        consumeTokenType(FOLParse::OpenParen, its);
        unsigned int k = consumeNumber(its);
        consumeTokenType(FOLParse::Comma, its);
        unsigned int l = consumeNumber(its);
        consumeTokenType(FOLParse::CloseParen, its);
        consumeTokenType(FOLParse::CloseBracket, its);

        return SpanInterval(Interval(i,j), Interval(k,l));
    }
}

template <class ForwardIterator>
SpanInterval doParseInterval3(unsigned int i, iters<ForwardIterator> &its) {
    if (peekTokenType(FOLParse::Comma, its)) {
        consumeTokenType(FOLParse::Comma, its);
        unsigned int k = consumeNumber(its );
        consumeTokenType(FOLParse::CloseBracket, its);
        return SpanInterval(Interval(i,i), Interval(k,k));
    } else {
        consumeTokenType(FOLParse::Colon, its);
        unsigned int k = consumeNumber(its);
        consumeTokenType(FOLParse::CloseBracket, its);
        return SpanInterval(Interval(i,k), Interval(i,k));
    }
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseEventLiteral(iters<ForwardIterator> &its) {
    if (peekTokenType(FOLParse::Not, its)) {
        consumeTokenType(FOLParse::Not, its);
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

    consumeTokenType(FOLParse::OpenParen, its);
    if (peekTokenType(FOLParse::CloseParen, its)) {
        // ok, empty atom, finish parsing
        consumeTokenType(FOLParse::CloseParen, its);
        return a;
    }
    std::auto_ptr<Term> c(new Constant(consumeIdent(its)));
    a->push_back(c); // ownership transfered to atom
    while (peekTokenType(FOLParse::Comma, its)) {
        consumeTokenType(FOLParse::Comma, its);
        std::auto_ptr<Term> cnext(new Constant(consumeIdent(its)));
        a->push_back(cnext);
    }
    consumeTokenType(FOLParse::CloseParen, its);

    return a;
}



template <class ForwardIterator>
boost::shared_ptr<Atom> doParseAtom(iters<ForwardIterator> &its) {
    std::string predName = consumeIdent(its);
    boost::shared_ptr<Atom> a(new Atom(predName));

    consumeTokenType(FOLParse::OpenParen, its);
    if (peekTokenType(FOLParse::Identifier, its)) {
        std::auto_ptr<Term> c(new Constant(consumeIdent(its)));
        a->push_back(c);
    } else if (peekTokenType(FOLParse::Variable, its)){
        std::auto_ptr<Term> v(new Variable(consumeVariable(its)));
        a->push_back(v);
    } else {
        // atom with no args
        consumeTokenType(FOLParse::CloseParen, its);
        return a;
    }
    while (peekTokenType(FOLParse::Comma, its)) {
        consumeTokenType(FOLParse::Comma, its);
        if (peekTokenType(FOLParse::Identifier, its)) {
            std::auto_ptr<Term> c(new Constant(consumeIdent(its)));
            a->push_back(c);
        } else {
            std::auto_ptr<Term> v(new Variable(consumeVariable(its)));
            a->push_back(v);
        }
    }
    consumeTokenType(FOLParse::CloseParen, its);
    return a;
}

template <class ForwardIterator>
ELSentence doParseWeightedFormula(iters<ForwardIterator> &its) {
    bool hasWeight;
    double weight;
    if (peekTokenType(FOLParse::Number, its)) {
        hasWeight = true;
        weight = consumeNumber(its);
        consumeTokenType(FOLParse::Colon, its);
    } else if (peekTokenType(FOLParse::Float, its)) {
        hasWeight = true;
        weight = consumeFloat(its);
        consumeTokenType(FOLParse::Colon, its);

        //consumeTokenType(FOLParse::Float, its);
    } else {
        if (peekTokenType(FOLParse::Infinity, its)) {
            consumeTokenType(FOLParse::Infinity, its);
            consumeTokenType(FOLParse::Colon, its);
        }
        hasWeight = false;
        weight = 0.0;
    }
    boost::shared_ptr<Sentence> p = doParseFormula(its);
    ELSentence sentence(p);
    if (hasWeight) {
        sentence.setWeight(weight);
    } else {
        sentence.setHasInfWeight(true);
    }
    // check to see if it's quantified
    if (peekTokenType(FOLParse::At, its)) {
        consumeTokenType(FOLParse::At, its);
        //SISet set;
        std::vector<SpanInterval> sis;
        if (peekTokenType(FOLParse::OpenBrace, its)) {
            consumeTokenType(FOLParse::OpenBrace, its);
            while (!peekTokenType(FOLParse::CloseBrace, its)) {
                //SpanInterval si = doParseInterval(its);
                // normalize it
                boost::optional<SpanInterval> normSi = doParseInterval(its).normalize();
                if (normSi) sis.push_back(*normSi);
                if (!peekTokenType(FOLParse::CloseBrace, its)) {
                    consumeTokenType(FOLParse::Comma, its);
                }
            }
            consumeTokenType(FOLParse::CloseBrace, its);
        } else {
            boost::optional<SpanInterval> normSi = doParseInterval(its).normalize();
            if (normSi) sis.push_back(*normSi);
        }
        if (sis.empty()) {
            sentence.removeQuantification();
        } else {
            find_max_interval maxIntFinder;
            std::copy(sis.begin(), sis.end(), std::ostream_iterator<SpanInterval>(std::cout, ", "));
            maxIntFinder = std::for_each(sis.begin(), sis.end(), maxIntFinder);
            SISet set(false, maxIntFinder.max);
            for (std::vector<SpanInterval>::const_iterator it = sis.begin(); it != sis.end(); it++) {
                set.add(*it);
            }
            sentence.setQuantification(set);
        }
    } else {
        sentence.removeQuantification();
    }
    return sentence;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula(iters<ForwardIterator> &its) {
    return doParseFormula_exat(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_exat(iters<ForwardIterator> &its) {  // TODO: support exactly 1/at most 1
    return doParseFormula_quant(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_quant(iters<ForwardIterator> &its) { // TODO: support quantification
    return doParseFormula_imp(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_imp(iters<ForwardIterator> &its) {
    boost::shared_ptr<Sentence> s = doParseFormula_or(its);
    while (peekTokenType(FOLParse::Implies, its)) {
        consumeTokenType(FOLParse::Implies, its);
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
    while (peekTokenType(FOLParse::Or, its)) {
        consumeTokenType(FOLParse::Or, its);
        boost::shared_ptr<Sentence> s2 = doParseFormula_and(its);
        boost::shared_ptr<Sentence> dis(new Disjunction(s,s2));
        s = dis;
    }
    return s;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_and(iters<ForwardIterator> &its) {
    boost::shared_ptr<Sentence> s = doParseFormula_unary(its);
    while (peekTokenType(FOLParse::And, its) || peekTokenType(FOLParse::Semicolon, its)) {
        std::set<Interval::INTERVAL_RELATION> rels;
        if (peekTokenType(FOLParse::Semicolon, its)) {
            consumeTokenType(FOLParse::Semicolon, its);
            rels.insert(Interval::MEETS);   // meets is the default in this case;
        } else {
            consumeTokenType(FOLParse::And, its);
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
    if (peekTokenType(FOLParse::Not, its)) {
        consumeTokenType(FOLParse::Not, its);
        boost::shared_ptr<Sentence> s = doParseFormula_unary(its);
        boost::shared_ptr<Sentence> neg(new Negation(s));
        return neg;
    } else if (peekTokenType(FOLParse::Diamond, its)) {
        consumeTokenType(FOLParse::Diamond, its);
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
    if (peekTokenType(FOLParse::OpenBrace, its)) {
        consumeTokenType(FOLParse::OpenBrace, its);
        if (peekTokenType(FOLParse::Star, its)) {
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
            consumeTokenType(FOLParse::Star, its);
        } else {
            relations.insert(doParseRelation(its));
        }
        while (peekTokenType(FOLParse::Comma, its)) {
            consumeTokenType(FOLParse::Comma, its);
            relations.insert(doParseRelation(its));
        }
        consumeTokenType(FOLParse::CloseBrace, its);
    }
    return relations;
}

template <class ForwardIterator>
Interval::INTERVAL_RELATION doParseRelation(iters<ForwardIterator>& its) {
    if (peekTokenType(FOLParse::Equals, its)) {
        consumeTokenType(FOLParse::Equals, its);
        return Interval::EQUALS;
    } else if (peekTokenType(FOLParse::GreaterThan, its)) {
        consumeTokenType(FOLParse::GreaterThan, its);
        return Interval::GREATERTHAN;
    } else if (peekTokenType(FOLParse::LessThan, its)) {
        consumeTokenType(FOLParse::LessThan, its);
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
            std::stringstream str;
            str << "no interval relation matches \"" << str << "\"" << std::endl;
            str << "line number: " << its.cur->lineNumber() << ", column number: " << its.cur->colNumber() << std::endl;
            p.details = str.str();
            throw p;
        }
    }
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseFormula_paren(iters<ForwardIterator> &its) {
    if (peekTokenType(FOLParse::OpenBracket, its)) {
        consumeTokenType(FOLParse::OpenBracket, its);
        boost::shared_ptr<Sentence> s = doParseStaticFormula(its);
        consumeTokenType(FOLParse::CloseBracket, its);

        boost::shared_ptr<Sentence> liq(new LiquidOp(s));
        return liq;
    } else if (peekTokenType(FOLParse::OpenParen, its)) {
        consumeTokenType(FOLParse::OpenParen, its);
        boost::shared_ptr<Sentence> s = doParseFormula(its);
        consumeTokenType(FOLParse::CloseParen, its);
        return s;
    } else if (peekTokenType(FOLParse::True, its)) {
        consumeTokenType(FOLParse::True, its);
        boost::shared_ptr<Sentence> s(new BoolLit(true));
        return s;
    } else if (peekTokenType(FOLParse::False, its)) {
        consumeTokenType(FOLParse::False, its);
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
boost::shared_ptr<Sentence> doParseStaticFormula_exat(iters<ForwardIterator> &its) {    // TODO: support exactly 1/at most 1
    return doParseStaticFormula_quant(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_quant(iters<ForwardIterator> &its) {   // TODO: support quantification
    return doParseStaticFormula_imp(its);
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_imp(iters<ForwardIterator> &its) { // TODO: support double-implication
    boost::shared_ptr<Sentence> s1 = doParseStaticFormula_or(its);
    while (peekTokenType(FOLParse::Implies, its)) { // Implication is just special case of disjunction, ie X -> Y = !X v Y
        consumeTokenType(FOLParse::Implies, its);
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
    while (peekTokenType(FOLParse::Or, its)) {
        consumeTokenType(FOLParse::Or, its);
        boost::shared_ptr<Sentence> s2 = doParseStaticFormula_and(its);
        boost::shared_ptr<Sentence> dis(new Disjunction(s1, s2));

        s1 = dis;
    }
    return s1;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_and(iters<ForwardIterator> &its) {
    boost::shared_ptr<Sentence> s1 = doParseStaticFormula_unary(its);
    while (peekTokenType(FOLParse::And, its)) {
            consumeTokenType(FOLParse::And, its);
            boost::shared_ptr<Sentence> s2 = doParseStaticFormula_unary(its);
            boost::shared_ptr<Sentence> con(new Conjunction(s1, s2));

            s1 = con;
    }
    return s1;
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_unary(iters<ForwardIterator> &its) {
    if (peekTokenType(FOLParse::Not, its)) {
        consumeTokenType(FOLParse::Not, its);
        boost::shared_ptr<Sentence> s(new Negation(doParseStaticFormula_paren(its)));
        return s;
    } else {
        return doParseStaticFormula_paren(its);
    }
}

template <class ForwardIterator>
boost::shared_ptr<Sentence> doParseStaticFormula_paren(iters<ForwardIterator> &its) {
    boost::shared_ptr<Sentence> s;
    if (peekTokenType(FOLParse::OpenParen, its)) {
        consumeTokenType(FOLParse::OpenParen, its);
        s = doParseStaticFormula(its);
        consumeTokenType(FOLParse::CloseParen, its);
    } else if (peekTokenType(FOLParse::True, its)) {
        consumeTokenType(FOLParse::True, its);
        boost::shared_ptr<Sentence> s(new BoolLit(true));
        return s;
    } else if (peekTokenType(FOLParse::False, its)) {
        consumeTokenType(FOLParse::False, its);
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

// TODO: change this from parsing FOL::Events to Proposition/SISet pairs
void parseEventFile(const std::string &filename, std::vector<FOL::Event>& store) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::runtime_error e("unable to open event file " + filename + " for parsing");
        throw e;
    }
    std::vector<FOLToken> tokens = FOLParse::tokenize(file);
    //BOOST_FOREACH(FOLToken token, tokens) {
    //  std::cout << "type: " << token.type() <<  " contents: " << token.contents() << std::endl;
    //}

    iters<std::vector<FOLToken>::const_iterator > its(tokens.begin(), tokens.end());
    try {
        doParseEvents(store, its);
    } catch (bad_parse& e) {
        e.details += "filename: " +filename + "\n";
        file.close();
        throw;
    }
    file.close();
};

void parseFormulaFile(const std::string &filename, std::vector<ELSentence>& store) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::runtime_error e("unable to open event file " + filename + " for parsing");
        throw e;
    }
    std::vector<FOLToken> tokens = FOLParse::tokenize(file);
    iters<std::vector<FOLToken>::const_iterator> its(tokens.begin(), tokens.end());
    try {
        doParseFormulas(store, its);
    } catch (bad_parse& e) {
        e.details += "filename: " + filename + "\n";
        file.close();
        throw;
    }
    file.close();
};

template <class ForwardIterator>
void parseFormulas(const ForwardIterator &first,
        const ForwardIterator &last, std::vector<ELSentence>& store) {
    iters<std::vector<FOLToken>::const_iterator> its(first, last);
    doParseFormulas(store, its);
}

Domain loadDomainFromFiles(const std::string &eventfile, const std::string &formulafile, const ParseOptions& options=ParseOptions()) {
    std::vector<FOL::Event> events;
    //std::vector<WSentence> formulas;
    std::vector<ELSentence> formSet;

    parseEventFile(eventfile, events);
    std::cout << "Read " << events.size() << " events from file." << std::endl;
    parseFormulaFile(formulafile, formSet);
    std::cout << "Read " << formSet.size() << " formulas from file." << std::endl;

    Domain d;
    boost::unordered_set<Atom> factAtoms;   // collect the fact atoms
    for (std::vector<FOL::Event>::const_iterator it = events.begin(); it != events.end(); it++) {
        // convert to proposition
        Proposition prop(*it->atom(), it->truthVal());
        factAtoms.insert(*it->atom());
        Interval maxInt(it->where().start().start(), it->where().finish().finish());
        SISet where(it->where(), true, maxInt);     // Locking all facts from the events file as liquid - need a better way to do this
        // TODO: type system!
        d.addFact(prop, where);
    }
    d.addFormulas(formSet.begin(), formSet.end());

    if (options.assumeClosedWorldInFacts()) {
        // for every fact atom, subtract the places where it's not true, and add in the false statements
        for (boost::unordered_set<Atom>::const_iterator it = factAtoms.begin(); it != factAtoms.end(); it++) {
            SISet noFixedValue = d.getModifiableSISet(*it);
            if (!noFixedValue.empty()) {
                Proposition prop(*it, false);
                d.addFact(prop, noFixedValue);
            }
        }
    }
    return d;

}

template <class ForwardIterator>
void parseEvents(const ForwardIterator &first,
        const ForwardIterator &last, std::vector<FOL::Event>& store) {
    iters<ForwardIterator> its(first, last);
    doParseEvents(store, its);
}

template <class ForwardIterator>
std::vector<FOL::Event> parseEvent(const ForwardIterator &first,
        const ForwardIterator &last) {
    iters<ForwardIterator> its(first, last);
    return doParseEvent(its);
}

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
