#ifndef FOLTOKEN_H
#define FOLTOKEN_H

#include <string>

namespace FOLParse {
enum TokenType { Invalid,
    OpenParen,
    CloseParen,
    Comma,
    Colon,
    Semicolon,
    Equals,
    GreaterThan,
    LessThan,
    Implies,
    DoubleImplies,
    At,
    And,
    Or,
    Not,
    Diamond,
    Exactly1,
    AtLeast1,
    EndLine,
    OpenBracket,
    CloseBracket,
    OpenBrace,
    CloseBrace,
    Identifier,
    Star,
    Variable,
    Var,
    Number,
    Float,
    True,
    False,
    Init,
    Infinity};
}

class FOLToken {
public:
    FOLToken(FOLParse::TokenType type=FOLParse::Invalid, std::string contents="");

    FOLParse::TokenType type() const {return typ;};
    void setType(FOLParse::TokenType type) {typ=type;};
    std::string contents() const {return data;};
    void setContents(std::string contents) {data = contents;};

private:
    FOLParse::TokenType typ;
    std::string data;
};

#endif
