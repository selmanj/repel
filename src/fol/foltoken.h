#ifndef FOLTOKEN_H
#define FOLTOKEN_H

#include <string>

namespace FOLParse {
enum FOL_TOKEN_TYPE { INVALID,
	OPEN_PAREN,
	CLOSE_PAREN,
	COMMA,
	COLON,
	SEMICOLON,
	EQUALS,
	GT,
	LT,
	IMPLIES,
	DOUBLEIMPLIES,
	AT,
	AND,
	OR,
	NOT,
	DIAMOND,
	EX1,
	AT1,
	ENDL,
	OPEN_BRACKET,
	CLOSE_BRACKET,
	OPEN_BRACE,
	CLOSE_BRACE,
	IDENT,
	VARIABLE,
	VAR,
	NUMBER,
	TTRUE,
	TFALSE};
}

class FOLToken {
public:
	FOLToken(FOLParse::FOL_TOKEN_TYPE type=FOLParse::INVALID, std::string contents="");

	FOLParse::FOL_TOKEN_TYPE type() const {return typ;};
	void setType(FOLParse::FOL_TOKEN_TYPE type) {typ=type;};
	std::string contents() const {return data;};
	void setContents(std::string contents) {data = contents;};

private:
	FOLParse::FOL_TOKEN_TYPE typ;
	std::string data;
};

#endif
