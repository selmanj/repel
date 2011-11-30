#ifndef TESTUTILITIES_H
#define TESTUTILITIES_H

#include <boost/shared_ptr.hpp>
#include "logic/fol.h"
#include "logic/folparser.h"
#include "logic/follexer.h"
#include "logic/formulaset.h"
#include <istream>

boost::shared_ptr<Sentence> getAsSentence(const std::string& str) {
	std::istringstream stream(str);
	std::vector<FOLToken> tokens = FOLParse::tokenize(&stream);
	return FOLParse::parseFormula(tokens.begin(),tokens.end());
}

Domain loadDomainWithStreams(const std::string& facts, const std::string& formulas) {
	std::istringstream factsStream(facts);
	std::istringstream formulasStream(formulas);

	std::vector<FOLToken> factsTokens = FOLParse::tokenize(&factsStream);
	std::vector<FOLToken> formulaTokens = FOLParse::tokenize(&formulasStream);

	std::vector<FOL::Event> factvec;
	FormulaSet formulaSet;

	FOLParse::parseEvents(factsTokens.begin(), factsTokens.end(), factvec);
	FOLParse::parseFormulas(formulaTokens.begin(), formulaTokens.end(), formulaSet);
	return Domain(factvec.begin(), factvec.end(), formulaSet);
}

#endif
