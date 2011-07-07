//============================================================================
// Name        : pel.cpp
// Author      : Joe Selman
// Version     :

//============================================================================

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifndef PACKAGE_STRING
#define PACKAGE_STRING "pel 0.8"
#endif

#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include <boost/foreach.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <ctime>
#include "fol/fol.h"
#include "fol/folparser.h"
#include "fol/domain.h"
#include "log.h"
#include "fol/moves.h"

int main(int argc, char* argv[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "produce help message")
	    ("version,v", "print version and exit")
	    ("max", po::value<unsigned int>(), "maximum value an interval endpoint can take")
	    ("min", po::value<unsigned int>(), "minimum value an interval endpoint can take")
	    ("evalModel,e", "simply print the model weight of the facts file")
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

	if (vm.count("version")) {
		std::cout << PACKAGE_STRING << std::endl;
		return 0;
	}

	if (vm.count("help") || !vm.count("facts-file") || !vm.count("formula-file")) {
	    std::cout << "Usage: pel [OPTION]... FACT-FILE FORMULA-FILE" << std::endl;
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
	if (vm.count("max") || vm.count("min")) {
		Interval maxInt = d->maxInterval();
		if (vm.count("max")) maxInt.setFinish(vm["max"].as<unsigned int>());
		if (vm.count("min")) maxInt.setStart(vm["min"].as<unsigned int>());
		d->setMaxInterval(maxInt);
	}

	Model model = d->defaultModel();

	LOG_PRINT(LOG_INFO) << "model size: " << model.size();
	LOG(LOG_DEBUG) << "observation predicates: ";
	BOOST_FOREACH(std::string name, d->observedPredicates()) {
		LOG(LOG_DEBUG) << "\t" << name;
	}

	if (vm.count("evalModel")) {
		LOG(LOG_INFO) << "evaluating model...";
		unsigned long sum = 0;
		// evaluate the weight of each formula in the domain
		BOOST_FOREACH(const WSentence formula, d->formulas()) {
			SISet satisfied = d->satisfied(*(formula.sentence()), model);
			unsigned long weight = d->score(formula, model);
			sum += weight;
			LOG_PRINT(LOG_INFO) << "formula: (" << formula.sentence()->toString() << ")";
			LOG_PRINT(LOG_INFO) << "\tsatisfied @ " << satisfied.toString();
			LOG_PRINT(LOG_INFO) << "\tscore contributed: " << weight;
		}
		LOG_PRINT(LOG_INFO) << "total score of model: " << sum;
	} else {
		double p = vm["prob"].as<double>();
		unsigned int iterations = vm["iterations"].as<unsigned int>();

		LOG(LOG_INFO) << "searching for a maximum-weight model, with p=" << p << " and iterations=" << iterations;
		Model defModel = d->defaultModel();
		Model maxModel = maxWalkSat(*d, iterations, p, &defModel);
		LOG_PRINT(LOG_INFO) << "Best model found: " << std::endl;
		LOG_PRINT(LOG_INFO) << modelToString(maxModel);
		if (vm.count("output")) {
			// log it to the output file as well
			fprintf(outputFile, "# generated from fact file \"%s\" and formula file \"%s\"\n",
					vm["facts-file"].as<std::string>().c_str(),
					vm["formula-file"].as<std::string>().c_str());
			std::string timeStr;
			{
				time_t rawtime;
				struct tm * timeinfo;
				time (&rawtime);
				timeinfo = localtime (&rawtime);
				timeStr = asctime(timeinfo);
			}
			fprintf(outputFile, "# generated on %s\n", timeStr.c_str());
			fprintf(outputFile, "# run with %d iterations and %g chance of choosing a random move\n",
					iterations,
					p);
			fputs(modelToString(maxModel).c_str(), outputFile);
		}
	}

	// Should be good and close files?
	return 0;
}
