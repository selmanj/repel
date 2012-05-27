#ifndef TERM_H
#define TERM_H

#include <string>
#include <boost/utility.hpp>
#include <iostream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>

class Term : boost::noncopyable {
public:
    virtual ~Term();

    std::string name() const;
    Term* clone() const;
    friend bool operator==(const Term& a, const Term& b);
    friend bool operator!=(const Term& a, const Term& b);

    std::string toString() const;

    friend std::size_t hash_value(const Term& t);

protected:
    virtual void doToString(std::string& str) const = 0;

private:

    virtual Term* doClone() const = 0;
    virtual std::string doName() const = 0;
    virtual bool doEquals(const Term& t) const = 0;
    virtual std::size_t doHash() const = 0;
};

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Term)
BOOST_CLASS_EXPORT_KEY(Term)

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

inline std::size_t hash_value(const Term& t) {
    return t.doHash();
}

inline Term* new_clone(const Term& t) {return t.clone();}

#endif
