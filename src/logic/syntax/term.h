#ifndef TERM_H
#define TERM_H

#include <string>
#include <boost/utility.hpp>
#include <iostream>

class Term : boost::noncopyable {
public:
    virtual ~Term();

    std::string name() const;
    Term* clone() const;
    friend bool operator==(const Term& a, const Term& b);
    friend bool operator!=(const Term& a, const Term& b);

    std::string toString() const;

protected:
    virtual void doToString(std::string& str) const = 0;

private:
    virtual Term* doClone() const = 0;
    virtual std::string doName() const = 0;
    virtual bool doEquals(const Term& t) const = 0;
};

// IMPLEMENTATION
inline Term::~Term() {};

inline std::string Term::name() const { return doName(); }
inline Term* Term::clone() const { return doClone(); }

inline bool operator==(const Term& a, const Term& b) { return a.doEquals(b);}
inline bool operator!=(const Term& a, const Term& b) { return !operator==(a,b);}

inline std::string Term::toString() const {
    std::string str;
    doToString(str);
    return str;
}

inline Term* new_clone(const Term& t) {return t.clone();}

#endif
