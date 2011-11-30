
#include <string>
#include "variable.h"

Variable::Variable(std::string name) 
: name_(name)
{
}

Term* Variable::doClone() const {
	return new Variable(*this);
}

bool Variable::doEquals(const Term& t) const {
	// first try to cast t to a var
	const Variable *var = dynamic_cast<const Variable*>(&t);
	if (var == NULL) {
		return false; // wrong type
	}
	return var->name_ == name_;
}

void Variable::doToString(std::string& str) const {
	str += "?";
	str += name_;
}
