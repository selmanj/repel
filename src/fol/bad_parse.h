#ifndef BAD_PARSE_H
#define BAD_PARSE_H

#include <exception>

class bad_parse : public std::exception {
	virtual const char* what() const throw() {
		return "Unable to parse"; // TODO: real clear there, this will surely stop all questions about failed parsings
	}
};

#endif
