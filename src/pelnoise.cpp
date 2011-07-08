/*
 * pelnoise.cpp
 *
 *  Created on: Jul 6, 2011
 *      Author: joe
 */


#include <boost/program_options.hpp>
namespace po = boost::program_options;
#include "pelnoise.h"
#include "fol/fol.h"
#include "fol/domain.h"
#include "fol/folparser.h"

Model constructModel(const std::vector<FOL::EventPair>& pairs) {\
	Model m;
	unsigned int smallest=UINT_MAX, largest=0;
	// find the max interval
	for (std::vector<FOL::EventPair>::const_iterator it = pairs.begin(); it != pairs.end(); it++) {
		SpanInterval interval = it->second;

		boost::optional<SpanInterval> norm = interval.normalize();
		if (!norm) {
			continue;
		}
		interval = norm.get();
		smallest = (std::min)(interval.start().start(), smallest);
		largest = (std::max)(interval.finish().finish(), largest);
	}

	Interval maxInterval = Interval(smallest, largest);

	// initialize observations
	for (std::vector<FOL::EventPair>::const_iterator it = pairs.begin(); it != pairs.end(); it++) {
		boost::shared_ptr<const Atom> atom = it->first;
		SpanInterval interval = it->second;

		// reinforce the max interval
		boost::optional<SpanInterval> opt = interval.setMaxInterval(maxInterval);
		if (!opt) continue;
		interval = opt.get();

		SISet set(true, maxInterval);

		set.add(interval);
		if (m.find(*atom) != m.end()) {
			set.add(m.find(*atom)->second);
			m.erase(*atom);
		}
		std::pair<Atom, SISet > pair(*atom, set);
		m.insert(pair);
	}
	return m;
}

Model subtractModel(const Model& from, const Model& toSubtract) {
	Model newModel;
	for (Model::const_iterator it = from.begin(); it != from.end(); it++) {
		Atom a = it->first;
		SISet set = it->second;

		if (toSubtract.find(a) != toSubtract.end()) {
			SISet setSubtract = toSubtract.find(a)->second;
			set.subtract(setSubtract);
		}
		if (set.size() != 0) newModel.insert(std::pair<Atom, SISet>(a, set));
	}
	return newModel;
}

Model intersectModel(const Model& a, const Model& b) {
	Model newModel;
	for (Model::const_iterator it = a.begin(); it != a.end(); it++) {
		Atom atom = it->first;
		SISet set = it->second;

		if (b.find(atom) != b.end()) {
			SISet intersect = intersection(set, b.find(atom)->second);
			if (intersect.size() != 0) newModel.insert(std::pair<Atom, SISet>(atom, intersect));
		}
	}
	return newModel;
}

Model complimentModel(const Model& a, const std::set<Atom, atomcmp>& allAtoms, const Interval& maxInterval) {
	Model newModel;
	BOOST_FOREACH(Atom atom, allAtoms) {
		if (a.find(atom) != a.end()) {
			Model::const_iterator it = a.find(atom);
			SISet set = it->second;
			if (set.compliment().size() != 0) {
				newModel.insert(std::pair<Atom, SISet>(atom, set.compliment()));
			}
		} else {
			SISet set(true, maxInterval);
			set.add(SpanInterval(maxInterval, maxInterval, maxInterval));
			newModel.insert(std::pair<Atom, SISet>(atom, set));
		}
	}

	return newModel;
}

Model rewritePELOutputAsConcreteModel(const Model& a) {
	Model newModel(a);
	// first, remove any atoms with D- in the front as those are old
	for (Model::const_iterator it = a.begin(); it != a.end(); it++) {
		Atom atom = it->first;
		SISet set = it->second;
		if (atom.name().find("D-") == 0) {
			newModel.erase(atom);
		}
	}
	// done?  ok now lets add D- to the front of all the atoms (weird but this will work)
	Model newModelCopy;
	for (Model::const_iterator it = newModel.begin(); it != newModel.end(); it++) {
		Atom atom = it->first;
		SISet set = it->second;

		atom.name() = std::string("D-") += atom.name();
		newModelCopy.insert(std::pair<Atom, SISet>(atom, set));
	}
	return newModelCopy;
}

unsigned int sizeModel(const Model& a) {
	unsigned int sum = 0;
	for (Model::const_iterator it = a.begin(); it != a.end(); it++) {
		sum += it->second.liqSize();
	}
	return sum;
}


int main(int argc, char* argv[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "produce help message")
	    ("batch,b", "run in batch mode, producing csv output")
	;

	po::options_description hidden("Hidden options");
	hidden.add_options()
		("ground-truth-file", po::value<std::string>(), "ground-truth file")
		("noisy-input-file", po::value<std::string>(), "noisy-input file")
		("noisy-output-file", po::value<std::string>(), "noisy-output file")

	;
	po::positional_options_description p;
	p.add("ground-truth-file", 1);
	p.add("noisy-input-file", 1);
	p.add("noisy-output-file", 1);

	po::options_description cmdline_options;
	cmdline_options.add(desc).add(hidden);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help") || !vm.count("ground-truth-file") || !vm.count("noisy-input-file") || !vm.count("noisy-output-file")) {
	    std::cout << "Usage: " << argv[0] <<"  GROUND-TRUTH-FILE NOISY-INPUT-FILE NOISY-OUTPUT-FILE" << std::endl;
		std::cout << desc << std::endl;
	    return 1;
	}
	std::vector<FOL::EventPair> truthFacts, noisyIn, noisyOut;

	FOLParse::parseEventFile(vm["ground-truth-file"].as<std::string>(), truthFacts);
	if (!vm.count("batch")) std::cout << "loaded ground truth file" << std::endl;
	FOLParse::parseEventFile(vm["noisy-input-file"].as<std::string>(), noisyIn);
	if (!vm.count("batch")) std::cout << "loaded noisy input file" << std::endl;
	FOLParse::parseEventFile(vm["noisy-output-file"].as<std::string>(), noisyOut);
	if (!vm.count("batch")) std::cout << "loaded noisy output file" << std::endl;

	// construct these as models
	Model truthModel = constructModel(truthFacts);
	Model noisyInModel = constructModel(noisyIn);
	Interval maxInterval = span(truthModel.begin()->second.maxInterval(), noisyInModel.begin()->second.maxInterval());

	Model noisyOutModel = constructModel(noisyOut);
	noisyOutModel = rewritePELOutputAsConcreteModel(noisyOutModel);

	std::set<Atom, atomcmp> allAtoms;

	for (Model::const_iterator it = truthModel.begin(); it != truthModel.end(); it++) {
		allAtoms.insert(it->first);
	}
	for (Model::const_iterator it = noisyInModel.begin(); it != noisyInModel.end(); it++) {
		allAtoms.insert(it->first);
	}
	for (Model::const_iterator it = noisyOutModel.begin(); it != noisyOutModel.end(); it++) {
		allAtoms.insert(it->first);
	}

	unsigned int noisyInTP = sizeModel(intersectModel(truthModel, noisyInModel));
	unsigned int noisyInFP = sizeModel(subtractModel(noisyInModel, truthModel));
	double noisyInPrecision = (double)noisyInTP / ((double)noisyInTP + (double)noisyInFP);


	unsigned int noisyInTN = sizeModel(intersectModel(complimentModel(truthModel, allAtoms, maxInterval), complimentModel(noisyInModel, allAtoms, maxInterval)));
	unsigned int noisyInFN = sizeModel(subtractModel(complimentModel(noisyInModel, allAtoms, maxInterval), complimentModel(truthModel, allAtoms, maxInterval)));
	double noisyInRecall = (double)noisyInTN / ((double)noisyInTN + (double)noisyInFN);

	unsigned int noisyOutTP = sizeModel(intersectModel(truthModel, noisyOutModel));
	unsigned int noisyOutFP = sizeModel(subtractModel(noisyOutModel, truthModel));
	double noisyOutPrecision = (double)noisyOutTP / ((double)noisyOutTP + (double)noisyOutFP);

	unsigned int noisyOutTN = sizeModel(intersectModel(complimentModel(truthModel, allAtoms, maxInterval), complimentModel(noisyOutModel, allAtoms, maxInterval)));
	unsigned int noisyOutFN = sizeModel(subtractModel(complimentModel(noisyOutModel, allAtoms, maxInterval), complimentModel(truthModel, allAtoms, maxInterval)));
	double noisyOutRecall = (double)noisyOutTN / ((double)noisyOutTN + (double)noisyOutFN);

	if (vm.count("batch")) {
		std::cout << noisyInTP << ", " << noisyInFP << ", " << noisyInPrecision << ", " << noisyInTN << ", " << noisyInFN << ", " << noisyInRecall << ", "
			<< noisyOutTP << ", " << noisyOutFP << ", " << noisyOutPrecision << ", " << noisyOutTN << ", " << noisyOutFN << ", " << noisyOutRecall << std::endl;
		return 0;
	}

	std::cout << "false positives in noisyInModel: " << modelToString(subtractModel(noisyInModel, truthModel)) << std::endl;
	std::cout << "false negatives in noisyInModel: " << modelToString(subtractModel(complimentModel(noisyInModel, allAtoms, maxInterval),
			complimentModel(truthModel, allAtoms, maxInterval))) << std::endl;

	std::cout << "noisyInTP = " << noisyInTP << ", noisyInFP = " << noisyInFP << ", noisyInPrecision = " << noisyInPrecision << std::endl;
	std::cout << "noisyInTN = " << noisyInTN << ", noisyInFN = " << noisyInFN << ", noisyInRecall = " << noisyInRecall << std::endl;

	std::cout << "false positives in noisyOutModel: " << modelToString(subtractModel(noisyOutModel, truthModel)) << std::endl;
	std::cout << "false negatives in noisyOutModel: " << modelToString(subtractModel(complimentModel(noisyOutModel, allAtoms, maxInterval),
			complimentModel(truthModel, allAtoms, maxInterval))) << std::endl;


	std::cout << "noisyOutTP = " << noisyOutTP << ", noisyOutFP = " << noisyOutFP << ", noisyOutPrecision = " << noisyOutPrecision << std::endl;
	std::cout << "noisyOutTN = " << noisyOutTN << ", noisyOutFN = " << noisyOutFN << ", noisyOutRecall = " << noisyOutRecall << std::endl;

	return 0;
}
