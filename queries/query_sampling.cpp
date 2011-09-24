/*
 * query_sampling.cpp
 *
 *  Created on: Aug 22, 2011
 *      Author: joe
 */
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <iostream>
#include "../src/log.h"

int main(int argc, char* argv[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		    		("help", "produce help message")
		    		("max", po::value<unsigned int>(), "maximum value an interval endpoint can take")
		    		("min", po::value<unsigned int>(), "minimum value an interval endpoint can take")
		    		("prob,p", po::value<double>()->default_value(0.25), "probability of taking a random move")
		    		("iterations,i", po::value<unsigned int>()->default_value(1000), "number of iterations before returning a model")
		    		("output,o", po::value<std::string>(), "output model file")
		    		;

	po::options_description hidden("Hidden options");
	hidden.add_options()
					("facts-file", po::value<std::string>(), "facts file")
					("formula-file", po::value<std::string>(), "formula file")
					;
	po::positional_options_description p;
	p.add("facts-file", 1);
	p.add("formula-file", 1);

	po::options_description cmdline_options;
	cmdline_options.add(desc).add(hidden);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help") || !vm.count("facts-file") || !vm.count("formula-file")) {
		std::cout << "Usage: query_sampling [OPTION]... FACT-FILE FORMULA-FILE" << std::endl;
		std::cout << desc << std::endl;
		return 1;
	}

	// setup our logging facilities
	FILE* debugFile = fopen("debug.log", "w");
	if (!debugFile) std::cerr << "unable to open debug.log for logging - logging to stderr";
	else FilePolicy::stream() = debugFile;
	FileLog::globalLogLevel() = LOG_DEBUG;

	LOG(LOG_INFO) << "Opened log file for new session";

	// make sure we can open the output model file, if specified
	FILE* outputFile = NULL;
	if (vm.count("output")) {
		outputFile = fopen(vm["output"].as<std::string>().c_str(), "w");
		if (!outputFile) {
			std::cerr << "unable to open output file \"" << vm["output"].as<std::string>() << "\" for writing." << std::endl;
			return 1;
		}
	}

	return 0;
}
