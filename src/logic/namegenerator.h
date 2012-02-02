/*
 * namegenerator.h
 *
 *  Created on: Jun 27, 2011
 *      Author: joe
 */

#ifndef NAMEGENERATOR_H_
#define NAMEGENERATOR_H_
#include <string>

#define NAME_PREFIX "__anonPred"

class NameGenerator {
public:
    NameGenerator();
    std::string getUniqueName();
private:
    unsigned int counter_;
};


#endif /* NAMEGENERATOR_H_ */
