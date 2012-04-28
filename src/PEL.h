/*
 * pelmap.h
 *
 *  Created on: Jan 25, 2012
 *      Author: joe
 */

#ifndef PELMAP_H_
#define PELMAP_H_

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include "logic/Domain.h"

int main(int argc, char* argv[]);

void initConfig(int argc,
        char* argv[],
        po::options_description& options,
        po::variables_map& vm);
#endif /* PELMAP_H_ */
