#ifndef CONSTANT_H
#define CONSTANT_H

#include <string>

class Constant : public Term {
  public:
    Constant(std::string name="");
    virtual ~Constant() {};

    std::string name() const {return str;};
  private:
    std::string str;
}

#endif
