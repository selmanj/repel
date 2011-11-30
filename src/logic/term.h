#ifndef TERM_H
#define TERM_H

#include <string>
#include <boost/utility.hpp>
#include <iostream>

class Term : boost::noncopyable {
public:
	virtual ~Term() {};
	std::string name() const { return doName(); };
	Term* clone() const { return doClone(); };
	bool operator==(const Term& b) const {
		return doEquals(b);};

	std::string toString() const {
		std::string str;
		doToString(str);
		return str;
	};

protected:
	virtual void doToString(std::string& str) const = 0;

private:
	virtual Term* doClone() const = 0;
	virtual std::string doName() const = 0;
	virtual bool doEquals(const Term& t) const = 0;
};

inline Term* new_clone(const Term& t) {
	return t.clone();
}
#endif
