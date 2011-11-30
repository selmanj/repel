#ifndef FOLLEXER_H
#define FOLLEXER_H

#include <vector>
#include <iostream>
#include "foltoken.h"
namespace FOLParse {
std::vector<FOLToken> tokenize(std::istream* input);
};

#endif
