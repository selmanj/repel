#ifndef FOLTOKEN_H
#define FOLTOKEN_H

#include <string>
#include <iostream>

// TODO: rename FOLParse and all its ilk to ELParse (event logic parse)
namespace FOLParse {
/**
 * TokenType represents a type of token when parsing event logic.  Each token
 * corresponds to some ASCII representation.
 */
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
    Infinity,
    Type};

    std::ostream& operator<<(std::ostream& out, FOLParse::TokenType token);
}

/**
 * A class representing a token resulting from running the lexer on an input
 * stream.  Tokens assist when parsing (and also contain some information
 * about the parse).
 */
class FOLToken {
public:
    /**
     * Construct a FOLToken of the given type and contents.  By default
     * (with no args) constructs an invalid token with no contents.
     *
     * @param type  The type of token.
     * @param contents  The contents of the token
     */
    FOLToken(FOLParse::TokenType type=FOLParse::Invalid,
            const std::string& contents="",
            unsigned int lineNumber=0,
            unsigned int colNumber=0)
        : type_(type),
          contents_(contents),
          lineNumber_(lineNumber),
          colNumber_(colNumber) {}

    /**
     * Get the type of the token.
     *
     * @return the type of the token.
     */
    FOLParse::TokenType type() const {return type_;};

    /**
     * Get the contents of the token.  The contents are an ASCII representation
     * of the token (ie a token of type comma has contents ",").  If the token
     * type is a number or identifier, the contents are a string representation
     * of the identifier or number.
     *
     * @return a string representation of the token type.
     */
    std::string contents() const {return contents_;};


    /**
     * Get the line number of the token.  A value of 0 indicates no line
     * number is known.
     *
     * @return the line number that the token was found at.
     */
    unsigned int lineNumber() const { return lineNumber_; }

    /**
     * Get the column number of the token.  A value of 0 indicates no column
     * number is known.
     *
     * @return the column number that the token was found at.
     */
    unsigned int colNumber() const { return colNumber_;}

    /**
     * Set the type of the token.
     *
     * @param type the new type of the token
     */
    void setType(FOLParse::TokenType type) {type_=type;};

    /**
     * Set the contents of the token.  This is an ASCII representation of the
     * type.
     *
     * @param contents The contents of the token.
     */
    void setContents(const std::string& contents) {contents_ = contents;};

    /**
     * Set the line number of the token.  A value of 0 indicates no line
     * number is known.
     *
     * @param number  the line number the token was discovered at.
     */
    void setLineNumber(unsigned int number) {lineNumber_ = number; }

    /**
     * Set the column number of the token.  A value of 0 indicates no column
     * number is known.
     *
     * @param number  the column number the token was discovered at.
     */
    void setColNumber(unsigned int number) {colNumber_ = number; }

private:
    FOLParse::TokenType type_;
    std::string contents_;

    unsigned int lineNumber_;
    unsigned int colNumber_;
};


#endif
