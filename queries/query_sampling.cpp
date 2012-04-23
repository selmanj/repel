/*
 * query_sampling.cpp
 *
 *  Created on: Aug 22, 2011
 *      Author: joe
 */
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <iostream>
#include "../src/Log.h"
#include "../src/fol/obsproxy.h"
#include "../src/fol/fol.h"
#include "../src/fol/FOLParser.h"

int main(int argc, char* argv[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		    		("help", "produce help message")
		    		("max", po::value<unsigned int>(), "maximum value an interval endpoint can take")
		    		("min", po::value<unsigned int>(), "minimum value an interval endpoint can take")
		    		("prob,p", po::value<double>()->default_value(0.25), "probability of taking a random move")
		    		("iterations,i", po::value<unsigned int>()->default_value(200), "number of iterations before returning a model")
		    		("output,o", po::value<std::string>(), "output model file")
		    		;

	po::options_description hidden("Hidden options");
	hidden.add_options()
					("facts-file", po::value<std::string>(), "facts file")
					("formula-file", po::value<std::string>(), "formula file")
					("query-file", po::value<std::string>(), "query file")
					;
	po::positional_options_description p;
	p.add("facts-file", 1);
	p.add("formula-file", 1);
	p.add("query-file", 1);

	po::options_description cmdline_options;
	cmdline_options.add(desc).add(hidden);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help") || !vm.count("facts-file") || !vm.count("formula-file") || !vm.count("query-file")) {
		std::cout << "Usage: query_sampling [OPTION]... FACT-FILE FORMULA-FILE QUERY-FILE" << std::endl;
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

	boost::shared_ptr<Domain> d = FOLParse::loadDomainFromFiles(vm["facts-file"].as<std::string>(), vm["formula-file"].as<std::string>());
	std::vector<FOL::Event> queries;	// restrict this to 1 for now
	FOLParse::parseEventFile(vm["query-file"].as<std::string>(), queries);
	if (queries.size() == 0) {
		LOG_PRINT(LOG_ERROR) << "no query found in query file!";
		return 1;
	}
	FOL::Event queryevent = queries.front();
	SISet set(false, d->maxInterval());
	set.add(queryevent.where());
	//std::pair<Atom, SISet> query(*queryevent.atom(), set);

	ObsProxy proxy(*d, *queryevent.atom(), set);
	std::vector<Model> samples = proxy.generateSamples();
	std::map<Model, int, modelcmp> clusters = proxy.kMedoid(2, samples);
	for (int i = 0; i < samples.size(); i++) {
		LOG_PRINT(LOG_INFO) << "Sample " << i << " assigned to cluster " << clusters[samples[i]];
	}

	return 0;
}
