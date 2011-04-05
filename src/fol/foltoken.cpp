
#include <string>
#include "foltoken.h"

FOLToken::FOLToken(FOLParse::FOL_TOKEN_TYPE type, std::string contents) 
  : typ(type), data(contents)
{
}

