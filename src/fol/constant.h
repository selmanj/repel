#ifndef CONSTANT_H
#define CONSTANT_H

#include <string>
#include "term.h"

class Constant : public Term {
public:
	Constant(std::string name);
	Constant(const Constant& c) : name_(c.name_) {};
	~Constant() {};

	void operator=(const Constant& c) {name_ = c.name_;};

protected:
	virtual void doToString(std::string& str) const {
		str += name_;
	}
private:
	virtual Term* doClone() const;
	virtual std::string doName() const {return name_;};
	virtual bool doEquals(const Term& t) const;
	std::string name_;
};

#endif
