#ifndef FOLTOKEN_H
#define FOLTOKEN_H

#include <string>

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
    Infinity};
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
    FOLToken(FOLParse::TokenType type=FOLParse::Invalid, const std::string& contents="")
        : type_(type), contents_(contents) {}

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

private:
    FOLParse::TokenType type_;
    std::string contents_;
};

#endif
