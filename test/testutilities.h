#ifndef TESTUTILITIES_H
#define TESTUTILITIES_H

#include <boost/shared_ptr.hpp>
#include "../src/fol/fol.h"
#include "../src/fol/folparser.h"
#include "../src/fol/follexer.h"
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

	std::vector<FOL::EventPair> factvec;
	std::vector<WSentence> formVec;

	FOLParse::parseEvents(factsTokens.begin(), factsTokens.end(), factvec);
	FOLParse::parseFormulas(formulaTokens.begin(), formulaTokens.end(), formVec);
	return Domain(factvec.begin(), factvec.end(), formVec.begin(), formVec.end());
}

#endif
