
#include <string>
#include "FOLToken.h"

FOLToken::FOLToken(FOLParse::TokenType type, std::string contents) 
: typ(type), data(contents)
{
}

