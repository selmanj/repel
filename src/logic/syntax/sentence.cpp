#include "sentence.h"
#include <sstream>

std::string TQConstraints::toString() const {
	std::stringstream str;

	if (mustBeIn.size() != 0) {
		str << "&" << mustBeIn.toString();
	}

	if (mustNotBeIn.size() != 0) {
		// only print a comma if we printed something before
		if (mustBeIn.size() != 0) {
			str << ",";
		}
		str << "\\" << mustNotBeIn.toString();
	}

	return str.str();
}
