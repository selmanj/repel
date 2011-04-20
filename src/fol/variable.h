#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include "term.h"

class Variable : public Term {
  public:
    Variable(std::string name);
    Variable(const Variable& c) : str(c.str) {};
    ~Variable() {};

    void operator=(const Variable& c) {str = c.str;};

  private:
    virtual Term* doClone() const;
    virtual std::string doName() const {return str;};
    virtual bool doEquals(const Term& t) const;
    std::string str;
};

#endif
