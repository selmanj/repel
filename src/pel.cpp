//============================================================================
// Name        : pel.cpp
// Author      : Joe Selman
// Version     :

//============================================================================

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "produce help message")
//	    ("i", po::value<std::string>(), "facts file")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
	    std::cout << desc << "\n";
	    return 1;
	}
/*
	if (vm.count("i")) {
	    std::cout << "input file was set to "
	 << vm["i"].as<std::string>() << ".\n";
	} else {
	    std::cout << "input file was not set.\n";
	}
	*/
	return 0;
}
