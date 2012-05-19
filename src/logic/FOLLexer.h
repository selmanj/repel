#ifndef FOLLEXER_H
#define FOLLEXER_H

#include <vector>
#include <iostream>

class FOLToken;

namespace FOLParse {
std::vector<FOLToken> tokenize(std::istream& input);
};

#endif
