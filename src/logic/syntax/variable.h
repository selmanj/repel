#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include "term.h"

class Variable : public Term {
public:
    Variable(std::string name);
    Variable(const Variable& c) : name_(c.name_) {};
    ~Variable() {};

    void operator=(const Variable& c) {name_ = c.name_;};
protected:
    void doToString(std::string& str) const;

private:
    virtual Term* doClone() const;
    virtual std::string doName() const {return name_;};
    virtual bool doEquals(const Term& t) const;
    std::string name_;
};

#endif
