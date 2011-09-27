/*
 * pelnoise.cpp
 *
 *  Created on: Jul 6, 2011
 *      Author: joe
 */


#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
namespace po = boost::program_options;
#include "pelnoise.h"
#include "fol/fol.h"
#include "fol/domain.h"
#include "fol/model.h"
#include "fol/folparser.h"

std::set<Atom, atomcmp> generateValidPreds() {
	std::string player_objs[] = {"backleft", "backright", "backmiddle", "frontleft", "frontright", "frontmiddle"};
	std::string player_preds[] = {"D-BallContact", "D-Spike", "D-Set", "D-Serve", "D-Dig", "D-Block", "D-Squat"};
	std::string ref_objs[] = {"refback", "refnet"};
	std::string ref_preds[] = {"D-Flag"};
	std::string noobj_preds[] = {"D-Huddle"};

	std::set<Atom, atomcmp> validPreds;
	BOOST_FOREACH(std::string pred, player_preds) {
		BOOST_FOREACH(std::string obj, player_objs) {
			boost::shared_ptr<Term> cons(new Constant(obj));
			Atom atom(pred, cons);
			validPreds.insert(atom);
		}
	}
	BOOST_FOREACH(std::string pred, ref_preds) {
		BOOST_FOREACH(std::string obj, ref_objs) {
			boost::shared_ptr<Term> cons(new Constant(obj));
			Atom atom(pred, cons);
			validPreds.insert(atom);
		}
	}
	BOOST_FOREACH(std::string pred, noobj_preds) {
		Atom atom(pred);
		validPreds.insert(atom);
	}

	// finally, dont forget about BallContact(them)
	{
		boost::shared_ptr<Term> them(new Constant("them"));
		Atom atom("D-BallContact", them);
		validPreds.insert(atom);
	}
	return validPreds;
}
/* NOTE!  Caller is responsible for deleting memory that was allocated for hte unsigned ints!  For
/* this tool its probably ok to not delete it as it will only run for a few ms but dont forget about this!
 */
ConfMatrix confusionMatrix(const Model& groundTruth, const Model& model, double threshhold) {
	ConfMatrix c;
	std::set<Atom, atomcmp> validPreds = generateValidPreds();
	// first initalize it
	BOOST_FOREACH(Atom a, validPreds) {
		BOOST_FOREACH(Atom b, validPreds) {
			std::pair<Atom, Atom> atomPair(a, b);
			unsigned int *counter = new unsigned int;
			*counter = 0;

			c.insert(std::pair<std::pair<Atom, Atom>, unsigned int* >(atomPair, counter));
		}
	}

	BOOST_FOREACH(Model::value_type pair, groundTruth) {
		Atom a = pair.first;
		SISet set = pair.second;
		if (c.find(std::pair<Atom, Atom>(a,a)) == c.end()) {
				continue;
		}

		BOOST_FOREACH(SpanInterval truthSi, set.set()) {
			// now, find ALL intervals that might intersect with htis
			std::map<Atom, SpanInterval, atomcmp > intersects;
			BOOST_FOREACH(Model::value_type modelPair, model) {
				Atom modelA = modelPair.first;
				SISet setA = modelPair.second;
				if (c.find(std::pair<Atom, Atom>(modelA,modelA)) == c.end()) {
					continue;
				}

				BOOST_FOREACH(SpanInterval modelSi, setA.set()) {
					if (intersection(modelSi, truthSi).size() > 0
							&& (double)intersection(modelSi, truthSi).liqSize() / ((double)truthSi.liqSize()) >= threshhold) {
						/*
						std::pair<Atom, Atom> atomPair(a, modelA);
						if (c.find(atomPair) == c.end()) {
							// do nothing
						} else {

							unsigned int *counter = c.at(atomPair);
							*counter = *counter + 1;
						}
						 */
						intersects.insert(std::pair<Atom, SpanInterval>(modelA, modelSi));
					}
				}
			}
			// ok we've got our list.  first, check to see if it contains our predicate anywhere
			if (intersects.find(a) != intersects.end()) {
				// we count this whole thing as positive
				unsigned int *counter = c.at(std::pair<Atom, Atom>(a, a));
				*counter = *counter+1;
				continue;
			} else if (intersects.size() == 0) {
				continue;	// do nothing!
			} else {
				// choose the highest valued intersection as the error and add 1 there
				double max = 0.0;
				std::pair<Atom, SpanInterval> bestFound = *intersects.begin();
				for (std::map<Atom, SpanInterval>::const_iterator it = intersects.begin();
						it != intersects.end();
						it++) {
					double size = (double)intersection(it->second, truthSi).liqSize() / ((double)truthSi.liqSize());
					if (size > max) {
						max = size;
						bestFound = *it;
					}
				}
				unsigned int *counter = c.at(std::pair<Atom, Atom>(a, bestFound.first));
				*counter = *counter+1;
			}

		}
	}
	return c;
}

boost::tuple<unsigned int, unsigned int, unsigned int, unsigned int> getThreshholdAccuracy(const Model& groundTruth, const Model& model, double threshhold) {
	unsigned int falsePos=0, truePos=0, trueNeg=0, falseNeg=0;
	BOOST_FOREACH(Model::value_type pair, groundTruth) {
		Atom a = pair.first;
		SISet set = pair.second;

		BOOST_FOREACH(SpanInterval si, set.set()) {
			SISet justSi(true, set.maxInterval());
			justSi.add(si);
			if (model.hasAtom(a)) {
				SISet modelSet = model.getAtom(a);
				SISet intersect = intersection(justSi, modelSet);
				bool truePosFound = false;
				BOOST_FOREACH(SpanInterval siInter, intersect.set()) {
					if (((double)siInter.liqSize() / (double) si.liqSize()) >= threshhold) {
						truePosFound = true;
						break;
					}
				}
				if (truePosFound) {
					truePos++;
				} else {
					falseNeg++;
				}
			} else {
				falseNeg++;
			}
		}
	}

	BOOST_FOREACH(Model::value_type pair, model) {
		Atom a = pair.first;
		SISet set = pair.second;

		BOOST_FOREACH(SpanInterval si, set.set()) {
			SISet justSi(true, set.maxInterval());
			justSi.add(si);
			if (groundTruth.hasAtom(a)) {
				SISet truthSet = groundTruth.getAtom(a);
				SISet intersect = intersection(justSi, truthSet);
				bool falsePosFound = true;
				BOOST_FOREACH(SpanInterval siInter, intersect.set()) {
					if (((double)siInter.liqSize() / (double) si.liqSize()) >= threshhold) {
						falsePosFound = false;
						break;
					}
				}
				if (falsePosFound) {
					falsePos++;
				}
			} else {
				falsePos++;
			}
		}
	}
	// now count the true negatives
	BOOST_FOREACH(Atom a, generateValidPreds()) {
		if (!groundTruth.hasAtom(a)
				&& !model.hasAtom(a)) {
			trueNeg++;
		} else if (model.hasAtom(a)) {
			// only in model
			bool trueNegFound = true;
			BOOST_FOREACH(SpanInterval si, model.getAtom(a).set()) {
				if ((double)si.liqSize() / (double)si.maxInterval().size() >= threshhold) {
					trueNegFound = false;
					break;
				}
			}
			if (trueNegFound) {
				trueNeg++;
			}
		} else if (groundTruth.hasAtom(a)) {
			// only in ground truth
			trueNeg = trueNeg + groundTruth.getAtom(a).compliment().set().size();
		} else {
			// it's in both
			BOOST_FOREACH(SpanInterval si, groundTruth.getAtom(a).compliment().set()) {
				SISet justSi(true, si.maxInterval());
				justSi.add(si);
				SISet intersect = intersection(justSi, model.getAtom(a));
				if (intersect.size() == 0) {
					trueNeg++;
				} else {
					bool trueNegFound = true;
					BOOST_FOREACH(SpanInterval si2, intersect.set()) {
						if ((double)si2.liqSize() / (double)si.liqSize() >= threshhold) {
							trueNegFound = false;
							break;
						}
					}
					if (trueNegFound) {
						trueNeg++;
					}
				}
			}

		}
	}

		/*
		BOOST_FOREACH(SpanInterval si, set.set()) {
			// find the set of intervals that intersect this one
			SISet intersects(true, set.maxInterval());
			// yes, this is inefficient but this is just a quick tool.  refactor this if its slow
			if (groundTruth.find(a) != groundTruth.end()) {
				SISet gtSet = groundTruth.find(a)->second;
				BOOST_FOREACH(SpanInterval gtsi, gtSet.set()) {
					if (intersection(si, gtsi).size() > 0) {
						intersects.add(gtsi);
					}
				}
			}
			// compute the ratio
			SISet justSet(true, set.maxInterval());
			justSet.add(si);
			unsigned int intersectSize = intersection(justSet, intersects).liqSize();
			std::cout << "intersectSize = " << intersectSize << std::endl;

			// recycle intersects value
			intersects.add(si);
			unsigned int unionSize = intersects.liqSize();
			std::cout << "unionSize = " << unionSize << std::endl;

			double ratio = (double)intersectSize / (double)unionSize;
			if (ratio < threshhold) {
				falsePos++;
			} else {
				truePos++;
			}
		}
		*/
	return boost::tuple<unsigned int, unsigned int, unsigned int, unsigned int>(truePos, falsePos, trueNeg, falseNeg);
}


Model rewritePELOutputAsConcreteModel(const Model& a) {
	Model newModel(a);
	// first, remove any atoms with D- in the front as those are old
	for (Model::const_iterator it = a.begin(); it != a.end(); it++) {
		Atom atom = it->first;
		SISet set = it->second;
		if (atom.name().find("D-") == 0) {
			newModel.clearAtom(atom);
		}
	}
	// done?  ok now lets add D- to the front of all the atoms (weird but this will work)
	Model newModelCopy;
	for (Model::const_iterator it = newModel.begin(); it != newModel.end(); it++) {
		Atom atom = it->first;
		SISet set = it->second;

		atom.name() = std::string("D-") += atom.name();
		newModelCopy.setAtom(atom, set);
	}
	return newModelCopy;
}


int main(int argc, char* argv[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "produce help message")
	    ("batch,b", "run in batch mode, producing csv output")
	    ("add,a", po::value<std::string>(), "add noise to file")
	    ("prob,p", po::value<double>(), "the percentage (expressed as a float from 0.0 to 1.0) of false intervals to add noise to")
	    ("thresh,t", po::value<double>()->default_value(0.5), "threshhold value for precision measure")
	    ("confusion,c", "generate confusion matrix only")
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

	if (vm.count("help") ||
			(!vm.count("add") && !vm.count("confusion") && (!vm.count("ground-truth-file") || !vm.count("noisy-input-file") || !vm.count("noisy-output-file")))
			|| (vm.count("add") && !vm.count("prob"))) {
		std::cout << "Usage:" << std::endl;
	    std::cout << "\t1. " << argv[0] <<" GROUND-TRUTH-FILE NOISY-INPUT-FILE NOISY-OUTPUT-FILE" << std::endl;
	    std::cout << "\t2. " << argv[0] <<" -p VALUE -a INPUT-FILE" << std::endl;
		std::cout << desc << std::endl;
	    return 1;
	}
	if (vm.count("add")) {
		std::vector<FOL::Event> inFile;

		FOLParse::parseEventFile(vm["add"].as<std::string>(), inFile);
		Model inModel(inFile);
		Interval maxInterval = inModel.begin()->second.maxInterval();

		std::set<Atom, atomcmp> validPreds = generateValidPreds();
		Model complModel(inModel);
		complModel.compliment(validPreds, maxInterval);
		double prob = vm["prob"].as<double>();
		unsigned long noiseToAdd = (double)complModel.size() * prob;
		if (!vm.count("batch")) std::cout << "noiseToAdd = " << noiseToAdd << std::endl;

		std::vector<Atom> validPredsVec(validPreds.begin(), validPreds.end());
		// choose a random predicate
		while (noiseToAdd != 0) {
			// choose a random predicate
			Atom toAdd = validPredsVec[rand() % validPredsVec.size()];

			if (!complModel.hasAtom(toAdd)) {
				continue; 	// wow now we can run forever!
			}
			SISet falseAt = complModel.getAtom(toAdd);

			if (falseAt.size() == 0) {
				continue;
			}
			// pick starting point
			unsigned long sizeStart = rand() % falseAt.liqSize();
			unsigned long start = 0;
			BOOST_FOREACH(SpanInterval si, falseAt.set()) {
				if (si.liqSize() < sizeStart) {
					// can't be here
					sizeStart = sizeStart - si.liqSize();
					continue;
				} else {
					start = si.start().start()+sizeStart;
				}
			}
			unsigned long length = rand() % noiseToAdd;
			unsigned long end = start+length;
			if (end > maxInterval.finish()) end = maxInterval.finish();
			// remember the size beforehand and after.  the amount it decreases is the true amount of noise added
			unsigned long modelSizeBefore = inModel.size();
			SISet trueAt;
			if (inModel.hasAtom(toAdd)) {
				trueAt = inModel.getAtom(toAdd);
				inModel.clearAtom(toAdd);
			} else {
				trueAt = SISet(true, maxInterval);
			}
			trueAt.add(SpanInterval(start, end, start, end, maxInterval));
			inModel.setAtom(toAdd, trueAt);
			unsigned long modelSizeAfter = inModel.size();
			// the difference is what we subtract from noiseToAdd
			noiseToAdd = noiseToAdd - (modelSizeAfter - modelSizeBefore);
			Model complModel(inModel);
			inModel.compliment(validPreds, maxInterval);
		}
		std::cout << inModel.toString();
	} else if (vm.count("confusion")) {
		std::vector<FOL::Event> truthFacts, inputFacts;

		FOLParse::parseEventFile(vm["ground-truth-file"].as<std::string>(), truthFacts);
		FOLParse::parseEventFile(vm["noisy-input-file"].as<std::string>(), inputFacts);
		Model truthModel(truthFacts);
		Model inputModel(inputFacts);

		ConfMatrix c = confusionMatrix(truthModel, inputModel, vm["thresh"].as<double>());
		// first find the set of predicates we are dealing with
		std::set<Atom, atomcmp> validPreds = generateValidPreds();
		// generate a heading to be nice
		for(std::set<Atom, atomcmp>::const_iterator it = validPreds.begin(); it != validPreds.end(); it++) {
			if (it != validPreds.begin()) {
				std::cout << ", ";
			}
			std::cout << "\"" << it->toString() << "\"";
		}
		std::cout << std::endl;

		for (std::set<Atom, atomcmp>::const_iterator aIt = validPreds.begin(); aIt != validPreds.end(); aIt++) {
			for (std::set<Atom, atomcmp>::const_iterator bIt = validPreds.begin(); bIt != validPreds.end(); bIt++) {
				if (bIt != validPreds.begin()) std::cout << ", ";

				std::pair<Atom, Atom> atomPair(*aIt, *bIt);
				if (c.find(atomPair) == c.end()) {
					std::cerr << "WARNING can't find a match for atompair " << aIt->toString() << ", " << bIt->toString() << std::endl;
					return 1;
				}
				std::cout << *c.at(atomPair);
			}
			std::cout << std::endl;
		}
		return 0;
	}
	else {

		std::vector<FOL::Event> truthFacts, noisyIn, noisyOut;

		FOLParse::parseEventFile(vm["ground-truth-file"].as<std::string>(), truthFacts);
		if (!vm.count("batch")) std::cout << "loaded ground truth file" << std::endl;
		FOLParse::parseEventFile(vm["noisy-input-file"].as<std::string>(), noisyIn);
		if (!vm.count("batch")) std::cout << "loaded noisy input file" << std::endl;
		FOLParse::parseEventFile(vm["noisy-output-file"].as<std::string>(), noisyOut);
		if (!vm.count("batch")) std::cout << "loaded noisy output file" << std::endl;

		// construct these as models
		Model truthModel(truthFacts);
		Model noisyInModel(noisyIn);
		Interval maxInterval = span(truthModel.begin()->second.maxInterval(), noisyInModel.begin()->second.maxInterval());

		Model noisyOutModel(noisyOut);
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

		unsigned int noisyInTP = intersectModel(truthModel, noisyInModel).size();
		unsigned int noisyInFP = subtractModel(noisyInModel, truthModel).size();
		double noisyInPrecision;
		if (!noisyInTP  && !noisyInFP ) {
			noisyInPrecision = 0.0;
		} else {
			noisyInPrecision = (double)noisyInTP / ((double)noisyInTP + (double)noisyInFP);
		}

		unsigned int noisyInTN = intersectModel(complimentModel(truthModel, allAtoms, maxInterval), complimentModel(noisyInModel, allAtoms, maxInterval)).size();
		unsigned int noisyInFN = subtractModel(complimentModel(noisyInModel, allAtoms, maxInterval), complimentModel(truthModel, allAtoms, maxInterval)).size();
		double noisyInRecall;
		if (!noisyInTP && !noisyInFN) {
			noisyInRecall = 0.0;
		} else {
			noisyInRecall = (double)noisyInTP / ((double)noisyInTP + (double)noisyInFN);
		}
		unsigned int noisyOutTP = intersectModel(truthModel, noisyOutModel).size();
		unsigned int noisyOutFP = subtractModel(noisyOutModel, truthModel).size();
		double noisyOutPrecision;
		if (!noisyOutTP && !noisyOutFP) {
			noisyOutPrecision = 0.0;
		} else {
			noisyOutPrecision = (double)noisyOutTP / ((double)noisyOutTP + (double)noisyOutFP);
		}

		unsigned int noisyOutTN = intersectModel(complimentModel(truthModel, allAtoms, maxInterval), complimentModel(noisyOutModel, allAtoms, maxInterval)).size();
		unsigned int noisyOutFN = subtractModel(complimentModel(noisyOutModel, allAtoms, maxInterval), complimentModel(truthModel, allAtoms, maxInterval)).size();
		double noisyOutRecall;
		if (!noisyOutTP && !noisyOutFN) {
			noisyOutRecall = 0.0;
		} else {
			noisyOutRecall = (double)noisyOutTP / ((double)noisyOutTP + (double)noisyOutFN);
		}
		boost::tuple<unsigned int, unsigned int, unsigned int, unsigned int> threshInPos = getThreshholdAccuracy(truthModel, noisyInModel, vm["thresh"].as<double>());
		unsigned int noisyInThreshTP = threshInPos.get<0>();
		unsigned int noisyInThreshFP = threshInPos.get<1>();

		double noisyInThreshPrecision;
		if (!noisyInThreshTP && !noisyInThreshFP) {
			noisyInThreshPrecision = 0.0;
		} else {
			noisyInThreshPrecision = (double)noisyInThreshTP / ((double)noisyInThreshTP + (double)noisyInThreshFP);
		}

		boost::tuple<unsigned int, unsigned int, unsigned int, unsigned int>  threshOutPos = getThreshholdAccuracy(truthModel, noisyOutModel, vm["thresh"].as<double>());
		unsigned int noisyOutThreshTP = threshOutPos.get<0>();
		unsigned int noisyOutThreshFP = threshOutPos.get<1>();
		unsigned int noisyOutThreshTN = threshOutPos.get<2>();
		unsigned int noisyOutThreshFN = threshOutPos.get<3>();

		double noisyOutThreshPrecision;
		if (!noisyOutThreshTP && !noisyOutThreshFP) {
			noisyOutThreshPrecision = 0.0;
		} else {
			noisyOutThreshPrecision = (double)noisyOutThreshTP / ((double)noisyOutThreshTP + (double)noisyOutThreshFP);
		}

		double noisyOutThreshRecall;
		if (!noisyOutThreshTP && !noisyOutThreshFN) {
			noisyOutThreshRecall = 0.0;
		} else {
			noisyOutThreshRecall = (double)noisyOutThreshTP / ((double)noisyOutThreshTP + (double)noisyOutThreshFN);
		}

		double noisyOutSpecificity;
		if (!noisyOutThreshFP && !noisyOutThreshTN) {
			noisyOutSpecificity = 0;
		} else {
			noisyOutSpecificity = (double)noisyOutThreshTN / ((double)noisyOutThreshTN + (double)noisyOutThreshFP);
		}


		if (vm.count("batch")) {
			std::cout << noisyInTP << ", " << noisyInFP << ", " << noisyInPrecision << ", " << noisyInTN << ", " << noisyInFN << ", " << noisyInRecall << ", "
				<< noisyOutTP << ", " << noisyOutFP << ", " << noisyOutPrecision << ", " << noisyOutTN << ", " << noisyOutFN << ", " << noisyOutRecall << ", "
				<< noisyInThreshTP << ", " << noisyInThreshFP << ", " << noisyInThreshPrecision << ", "
				<< noisyOutThreshTP << ", " << noisyOutThreshFP << ", " << noisyOutThreshPrecision << ", "
				<< noisyOutThreshTN << ", " << noisyOutThreshFN << ", " << noisyOutThreshRecall << ", "
				<< 1.0-noisyOutSpecificity << std::endl;
			return 0;
		}

		std::cout << "false positives in noisyInModel: " << subtractModel(noisyInModel, truthModel).size() << std::endl;
		std::cout << "false negatives in noisyInModel: " << subtractModel(complimentModel(noisyInModel, allAtoms, maxInterval),
				complimentModel(truthModel, allAtoms, maxInterval)).size() << std::endl;

		std::cout << "noisyInTP = " << noisyInTP << ", noisyInFP = " << noisyInFP << ", noisyInPrecision = " << noisyInPrecision << std::endl;
		std::cout << "noisyInTN = " << noisyInTN << ", noisyInFN = " << noisyInFN << ", noisyInRecall = " << noisyInRecall << std::endl;

		std::cout << "false positives in noisyOutModel: " << subtractModel(noisyOutModel, truthModel).size() << std::endl;
		std::cout << "false negatives in noisyOutModel: " << subtractModel(complimentModel(noisyOutModel, allAtoms, maxInterval),
				complimentModel(truthModel, allAtoms, maxInterval)).size() << std::endl;

		std::cout << "noisyOutTP = " << noisyOutTP << ", noisyOutFP = " << noisyOutFP << ", noisyOutPrecision = " << noisyOutPrecision << std::endl;
		std::cout << "noisyOutTN = " << noisyOutTN << ", noisyOutFN = " << noisyOutFN << ", noisyOutRecall = " << noisyOutRecall << std::endl;

		std::cout << "noisyInThreshTP = " << noisyInThreshTP << ", noisyInThreshFP = " << noisyInThreshFP << ", noisyInThreshPrecision = " << noisyInThreshPrecision << std::endl;
		std::cout << "noisyOutThreshTP = " << noisyOutThreshTP << ", noisyOutThreshFP = " << noisyOutThreshFP << ", noisyOutThreshPrecision = " << noisyOutThreshPrecision << std::endl;
		std::cout << "noisyOutThreshTN = " << noisyOutThreshTN << ", noisyOutThreshFN = " << noisyOutThreshFN << ", noisyOutThreshRecall = " << noisyOutThreshRecall << std::endl;

	}
	return 0;
}
