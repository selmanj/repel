
#include "FOLToken.h"

namespace FOLParse {
    std::ostream& operator<<(std::ostream& out, FOLParse::TokenType token) {
        // is there a better way than an explicit switch here?  :(
        switch (token) {
        case FOLParse::Invalid:
            out << "Invalid/Unknown";
            break;
        case FOLParse::OpenParen:
            out << "(";
            break;
        case FOLParse::CloseParen:
            out << ")";
            break;
        case FOLParse::Comma:
            out << ",";
            break;
        case FOLParse::Semicolon:
            out << ";";
            break;
        case FOLParse::Equals:
            out << "=";
            break;
        case FOLParse::GreaterThan:
            out << ">";
            break;
        case FOLParse::LessThan:
            out << "<";
            break;
        case FOLParse::Implies:
            out << "->";
            break;
        case FOLParse::DoubleImplies:
            out << "<->";
            break;
        case FOLParse::At:
            out << "@";
            break;
        case FOLParse::And:
            out << "And (& or ^)";
            break;
        case FOLParse::Or:
            out << "Or (v or |)";
            break;
        case FOLParse::Not:
            out << "Not (! or ~)";
            break;
        case FOLParse::Diamond:
            out << "Diamond (<>)";
            break;
        case FOLParse::Exactly1:
            out << "Exactly1 (ex1)";
            break;
        case FOLParse::AtLeast1:
            out << "AtLeast1 (at1)";
            break;
        case FOLParse::EndLine:
            out << "End line (\\n)";
            break;
        case FOLParse::OpenBracket:
            out << "[";
            break;
        case FOLParse::CloseBracket:
            out << "]";
            break;
        case FOLParse::OpenBrace:
            out << "{";
            break;
        case FOLParse::CloseBrace:
            out << "}";
            break;
        case FOLParse::Identifier:
            out << "Identifier (alphanumeric word)";
            break;
        case FOLParse::Star:
            out << "*";
            break;
        case FOLParse::Variable:
            out << "Variable (alphanumeric word)";
            break;
        case FOLParse::Var:
            out << "Keyword \"var\"";
            break;
        case FOLParse::Number:
            out << "Number ([0-9]*)";
            break;
        case FOLParse::Float:
            out << "Floating point number";
            break;
        case FOLParse::True:
            out << "Keyword \"true\"";
            break;
        case FOLParse::False:
            out << "Keyword \"false\"";
            break;
        case FOLParse::Init:
            out << "Keyword \"init\"";
            break;
        case FOLParse::Infinity:
            out << "Infinity (inf or infinity)";
            break;
        default:
            out << "Unknown token value: " << token;
            break;
        }

        return out;
    }
}
