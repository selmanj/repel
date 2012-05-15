
#include <vector>
#include <iostream>
#include <string>
#include "FOLLexer.h"
#include "FOLToken.h"

// TODO: make this use an iterator rather than return a vector
std::vector<FOLToken> FOLParse::tokenize(std::istream& input) {
    std::vector<FOLToken> tokens;
    while (!input.eof()) {
        int c = input.get();
        if (input.eof()) {
            break;
        }
        FOLToken token;
        // first check for identifiers
        if ((c >= 'A' && c <= 'Z')
                || (c >= 'a' && c <= 'z')
                || c == '_') {
            std::string ident;
            ident.push_back(c);
            while (!input.eof()) {
                c = input.peek();
                if ((c >= 'A' && c <= 'Z')
                        || (c >= 'a' && c <= 'z')
                        || (c >= '0' && c <= '9')
                        || c == '_'
                        || c == '-') {
                    input.get();
                    ident.push_back(c);
                } else {
                    break;
                }
            }
            // now check for reserved keywords
            if (ident == "v") {
                // OR
                token.setType(FOLParse::Or);
            } else if (ident == "var") {
                // VAR
                token.setType(FOLParse::Var);
            } else if (ident == "ex1") {
                // EX1
                token.setType(FOLParse::Exactly1);
            } else if (ident == "at1") {
                // AT1
                token.setType(FOLParse::AtLeast1);
            } else if (ident == "true") {
                // TRUE
                token.setType(FOLParse::True);
            } else if (ident == "false") {
                // FALSE
                token.setType(FOLParse::False);
            } else if (ident == "init") {
                // INIT
                token.setType(FOLParse::Init);
            } else if (ident == "inf") {
                // INF
                token.setType(FOLParse::Infinity);
            } else {
                // IDENT
                token.setType(FOLParse::Identifier);
            }
            token.setContents(ident);
            tokens.push_back(token);
        } else if ((c >= '0' && c <= '9') || c == '.') {
            std::string num;
            bool isFloat = (c == '.');
            num.push_back(c);
            while (!input.eof()) {
                c = input.peek();
                if ((c >= '0' && c <= '9') || c == '.') {
                    if (!isFloat && c == '.') isFloat = true;
                    input.get();
                    num.push_back(c);
                } else {
                    break;
                }
            }
            token.setType(isFloat ? FOLParse::Float : FOLParse::Number);
            token.setContents(num);
            tokens.push_back(token);
        } else if (c == '?') {
            // variable
            std::string var;
            while (!input.eof()) {
                c = input.peek();
                if ((c >= 'A' && c <= 'Z')
                        || (c >= 'a' && c <= 'z')
                        || (c >= '0' && c <= '9')
                        || c == '_') {
                    input.get();
                    var.push_back(c);
                } else {
                    break;
                }
            }
            token.setType(FOLParse::Variable);
            token.setContents(var);
            tokens.push_back(token);
        } else if ((c == '-' && input.peek() == '>')
                || (c == '=' && input.peek() == '>')) {
            input.get();
            // implies
            token.setType(FOLParse::Implies);
            token.setContents("->");
            tokens.push_back(token);
        } else if (c == '<' && input.peek() == '>') {
            input.get();
            // diamond
            token.setType(FOLParse::Diamond);
            token.setContents("<>");
            tokens.push_back(token);
        } else {
            switch (c) {    // simple cases below
            case '!':
            case '~':
                token.setType(FOLParse::Not);
                token.setContents("!");
                tokens.push_back(token);
                break;
            case '[':
                token.setType(FOLParse::OpenBracket);
                token.setContents("[");
                tokens.push_back(token);
                break;
            case ']':
                token.setType(FOLParse::CloseBracket);
                token.setContents("]");
                tokens.push_back(token);
                break;
            case '{':
                token.setType(FOLParse::OpenBrace);
                token.setContents("{");
                tokens.push_back(token);
                break;
            case '}':
                token.setType(FOLParse::CloseBrace);
                token.setContents("}");
                tokens.push_back(token);
                break;
            case '*':
                token.setType(FOLParse::Star);
                token.setContents("*");
                tokens.push_back(token);
                break;
            case '(':
                token.setType(FOLParse::OpenParen);
                token.setContents("(");
                tokens.push_back(token);
                break;
            case ')':
                token.setType(FOLParse::CloseParen);
                token.setContents(")");
                tokens.push_back(token);
                break;
            case '^':
            case '&':
                token.setType(FOLParse::And);
                token.setContents("^");
                tokens.push_back(token);
                break;
            case '|':
                token.setType(FOLParse::Or);
                token.setContents("v");
                tokens.push_back(token);
                break;
            case ',':
                token.setType(FOLParse::Comma);
                token.setContents(",");
                tokens.push_back(token);
                break;
            case ':':
                token.setType(FOLParse::Colon);
                token.setContents(":");
                tokens.push_back(token);
                break;
            case ';':
                token.setType(FOLParse::Semicolon);
                token.setContents(";");
                tokens.push_back(token);
                break;
            case '@':
                token.setType(FOLParse::At);
                token.setContents("@");
                tokens.push_back(token);
                break;
            case '=':
                token.setType(FOLParse::Equals);
                token.setContents("=");
                tokens.push_back(token);
                break;
            case '<':
                token.setType(FOLParse::LessThan);
                token.setContents("<");
                tokens.push_back(token);
                break;
            case '>':
                token.setType(FOLParse::GreaterThan);
                token.setContents(">");
                tokens.push_back(token);
                break;
            case '\n':
                token.setType(FOLParse::EndLine);
                token.setContents("\n");
                tokens.push_back(token);
                break;
            case '#':
                // comment, do nothing until we get to endl
                while (!input.eof()) {
                    c = input.peek();
                    if (c != '\n') {
                        input.get();
                    } else {
                        break;
                    }
                }
                break;
            case ' ':
            case '\t':
            case '\r':
                // do nothing!
                break;
            default:
                std::cerr << "dont know what " << c << " is" << std::endl;
                break;
                // error!
            }
        }
    }

    return tokens;
}
