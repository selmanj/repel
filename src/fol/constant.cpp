
#include <string>
#include "constant.h"

Constant::Constant(std::string name) 
: name_(name)
{
}

Term* Constant::doClone() const {
	return new Constant(*this);
}

bool Constant::doEquals(const Term& t) const {
	// first try to cast t to a constant
	const Constant *con = dynamic_cast<const Constant*>(&t);
	if (con == NULL) {
		return false; // wrong type
	}
	return con->name_ == name_;
}
