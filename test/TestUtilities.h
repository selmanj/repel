#ifndef TESTUTILITIES_H
#define TESTUTILITIES_H

#include <boost/shared_ptr.hpp>
#include "logic/ELSyntax.h"
#include "logic/FOLParser.h"
#include "logic/FOLLexer.h"
#include <istream>

Domain loadDomainWithStreams(const std::string& facts, const std::string& formulas, const ParseOptions& options=ParseOptions());
boost::shared_ptr<Sentence> getAsSentence(const std::string& str);

boost::shared_ptr<Sentence> getAsSentence(const std::string& str) {
    std::istringstream stream(str);
    std::vector<FOLToken> tokens = FOLParse::tokenize(&stream);
    return FOLParse::parseFormula(tokens.begin(),tokens.end());
}

Domain loadDomainWithStreams(const std::string& facts, const std::string& formulas, const ParseOptions& options) {
    std::istringstream factsStream(facts);
    std::istringstream formulasStream(formulas);

    std::vector<FOLToken> factsTokens = FOLParse::tokenize(&factsStream);
    std::vector<FOLToken> formulaTokens = FOLParse::tokenize(&formulasStream);

    std::vector<FOL::Event> factvec;
    std::vector<ELSentence> formulaSet;

    FOLParse::parseEvents(factsTokens.begin(), factsTokens.end(), factvec);
    FOLParse::parseFormulas(formulaTokens.begin(), formulaTokens.end(), formulaSet);

    // convert to propositions
    Domain d;
    boost::unordered_set<Atom> factAtoms;   // collect the fact atoms
    for (std::vector<FOL::Event>::const_iterator it = factvec.begin(); it != factvec.end(); it++) {
        factAtoms.insert(*it->atom());
        // convert to proposition
        Proposition prop(*it->atom(), it->truthVal());
        Interval maxInt(it->where().start().start(), it->where().finish().finish());
        SISet where(it->where(), true, maxInt);     // Locking all facts from the events file as liquid - need a better way to do this
        d.addFact(prop, where);
    }

    d.addFormulas(formulaSet.begin(), formulaSet.end());

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

#endif
