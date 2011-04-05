#ifndef FOLLEXER_H
#define FOLLEXER_H

#include <vector>
#include <iostream>
#include "foltoken.h"

// TODO: this is stored in an object, but its not clear that it needs to be
class FOLLexer {
public: 
  std::vector<FOLToken> tokenize(std::istream* input) const;
};

#endif
