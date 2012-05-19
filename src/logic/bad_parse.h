#ifndef BAD_PARSE_H
#define BAD_PARSE_H

#include <exception>
#include <string>

class bad_parse : public std::exception {
public:
    bad_parse() throw () {};
    bad_parse(bad_parse& other) : details(other.details) {};
    virtual ~bad_parse() throw () {};
    virtual const char* what() const throw() {return "unable to parse";};

    std::string details;
};

#endif
