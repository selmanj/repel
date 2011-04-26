#ifndef BAD_NORMALIZE_H
#define BAD_NORMALIZE_H

#include <exception>

class bad_normalize : public std::exception {
public:

	virtual const char* what() const throw() {
		return "Cannot normalize spanning interval without an empty set resulting.";
	}
};

#endif
