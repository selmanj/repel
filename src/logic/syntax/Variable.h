#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include <boost/functional/hash.hpp>
#include "Term.h"

class Variable : public Term {
public:
    Variable(std::string name);
    Variable(const Variable& c) : name_(c.name_) {};
    ~Variable() {};

    void operator=(const Variable& c) {name_ = c.name_;};

    friend std::size_t hash_value(const Variable& v);
protected:
    void doToString(std::string& str) const;

private:
    virtual Term* doClone() const;
    virtual std::string doName() const {return name_;};
    virtual bool doEquals(const Term& t) const;
    virtual std::size_t doHash() const;
    std::string name_;
};

inline std::size_t Variable::doHash() const {return hash_value(*this);}

// IMPLEMENTATION
inline std::size_t hash_value(const Variable& v) {
    boost::hash<std::string> hasher;
    return hasher(v.name_);
}
#endif
