
#include <vector>
#include <iostream>
#include <string>
#include "follexer.h"
#include "foltoken.h"

// TODO: make this use an iterator rather than return a vector
std::vector<FOLToken> FOLParse::tokenize(std::istream* input) {
    std::vector<FOLToken> tokens;
    while (!input->eof()) {
        int c = input->get();
        if (input->eof()) {
            break;
        }
        FOLToken token;
        // first check for identifiers
        if ((c >= 'A' && c <= 'Z')
                || (c >= 'a' && c <= 'z')
                || c == '_') {
            std::string ident;
            ident.push_back(c);
            while (!input->eof()) {
                c = input->peek();
                if ((c >= 'A' && c <= 'Z')
                        || (c >= 'a' && c <= 'z')
                        || (c >= '0' && c <= '9')
                        || c == '_'
                        || c == '-') {
                    input->get();
                    ident.push_back(c);
                } else {
                    break;
                }
            }
            // now check for reserved keywords
            if (ident == "v") {
                // OR
                token.setType(FOLParse::OR);
            } else if (ident == "var") {
                // VAR
                token.setType(FOLParse::VAR);
            } else if (ident == "ex1") {
                // EX1
                token.setType(FOLParse::EX1);
            } else if (ident == "at1") {
                // AT1
                token.setType(FOLParse::AT1);
            } else if (ident == "true") {
                // TRUE
                token.setType(FOLParse::TTRUE);
            } else if (ident == "false") {
                // FALSE
                token.setType(FOLParse::TFALSE);
            } else if (ident == "init") {
                // INIT
                token.setType(FOLParse::INIT);
            } else if (ident == "inf") {
                // INF
                token.setType(FOLParse::INF);
            } else {
                // IDENT
                token.setType(FOLParse::IDENT);
            }
            token.setContents(ident);
            tokens.push_back(token);
        } else if ((c >= '0' && c <= '9') || c == '.') {
            std::string num;
            bool isFloat = (c == '.');
            num.push_back(c);
            while (!input->eof()) {
                c = input->peek();
                if ((c >= '0' && c <= '9') || c == '.') {
                    if (!isFloat && c == '.') isFloat = true;
                    input->get();
                    num.push_back(c);
                } else {
                    break;
                }
            }
            token.setType(isFloat ? FOLParse::FLOAT : FOLParse::NUMBER);
            token.setContents(num);
            tokens.push_back(token);
        } else if (c == '?') {
            // variable
            std::string var;
            while (!input->eof()) {
                c = input->peek();
                if ((c >= 'A' && c <= 'Z')
                        || (c >= 'a' && c <= 'z')
                        || (c >= '0' && c <= '9')
                        || c == '_') {
                    input->get();
                    var.push_back(c);
                } else {
                    break;
                }
            }
            token.setType(FOLParse::VARIABLE);
            token.setContents(var);
            tokens.push_back(token);
        } else if ((c == '-' && input->peek() == '>')
                || (c == '=' && input->peek() == '>')) {
            input->get();
            // implies
            token.setType(FOLParse::IMPLIES);
            token.setContents("->");
            tokens.push_back(token);
        } else if (c == '<' && input->peek() == '>') {
            input->get();
            // diamond
            token.setType(FOLParse::DIAMOND);
            token.setContents("<>");
            tokens.push_back(token);
        } else {
            switch (c) {    // simple cases below
            case '!':
            case '~':
                token.setType(FOLParse::NOT);
                token.setContents("!");
                tokens.push_back(token);
                break;
            case '[':
                token.setType(FOLParse::OPEN_BRACKET);
                token.setContents("[");
                tokens.push_back(token);
                break;
            case ']':
                token.setType(FOLParse::CLOSE_BRACKET);
                token.setContents("]");
                tokens.push_back(token);
                break;
            case '{':
                token.setType(FOLParse::OPEN_BRACE);
                token.setContents("{");
                tokens.push_back(token);
                break;
            case '}':
                token.setType(FOLParse::CLOSE_BRACE);
                token.setContents("}");
                tokens.push_back(token);
                break;
            case '*':
                token.setType(FOLParse::STAR);
                token.setContents("*");
                tokens.push_back(token);
                break;
            case '(':
                token.setType(FOLParse::OPEN_PAREN);
                token.setContents("(");
                tokens.push_back(token);
                break;
            case ')':
                token.setType(FOLParse::CLOSE_PAREN);
                token.setContents(")");
                tokens.push_back(token);
                break;
            case '^':
            case '&':
                token.setType(FOLParse::AND);
                token.setContents("^");
                tokens.push_back(token);
                break;
            case '|':
                token.setType(FOLParse::OR);
                token.setContents("v");
                tokens.push_back(token);
                break;
            case ',':
                token.setType(FOLParse::COMMA);
                token.setContents(",");
                tokens.push_back(token);
                break;
            case ':':
                token.setType(FOLParse::COLON);
                token.setContents(":");
                tokens.push_back(token);
                break;
            case ';':
                token.setType(FOLParse::SEMICOLON);
                token.setContents(";");
                tokens.push_back(token);
                break;
            case '@':
                token.setType(FOLParse::AT);
                token.setContents("@");
                tokens.push_back(token);
                break;
            case '=':
                token.setType(FOLParse::EQUALS);
                token.setContents("=");
                tokens.push_back(token);
                break;
            case '<':
                token.setType(FOLParse::LT);
                token.setContents("<");
                tokens.push_back(token);
                break;
            case '>':
                token.setType(FOLParse::GT);
                token.setContents(">");
                tokens.push_back(token);
                break;
            case '\n':
                token.setType(FOLParse::ENDL);
                token.setContents("\n");
                tokens.push_back(token);
                break;
            case '#':
                // comment, do nothing until we get to endl
                while (!input->eof()) {
                    c = input->peek();
                    if (c != '\n') {
                        input->get();
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
