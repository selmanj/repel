#ifndef BAD_PARSE_H
#define BAD_PARSE_H

#include <exception>
#include <iostream>

class bad_parse : public std::exception {
public:
    bad_parse() throw () {};
    bad_parse(bad_parse& other) : details(other.details.str()) {};
    virtual ~bad_parse() throw () {};
    std::ostringstream details;
    virtual const char* what() const throw() {return "unable to parse";};
};

#endif
