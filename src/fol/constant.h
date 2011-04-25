#ifndef CONSTANT_H
#define CONSTANT_H

#include <string>
#include "term.h"

class Constant : public Term {
public:
	Constant(std::string name);
	Constant(const Constant& c) : str(c.str) {};
	~Constant() {};

	void operator=(const Constant& c) {str = c.str;};

private:
	virtual Term* doClone() const;
	virtual std::string doName() const {return str;};
	virtual bool doEquals(const Term& t) const;
	std::string str;
};

#endif
