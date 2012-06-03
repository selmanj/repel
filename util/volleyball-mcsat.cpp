/*
 * volleyball-mcsat.cpp
 *
 *  Created on: Jun 1, 2012
 *      Author: selman.joe@gmail.com
 */
#include <boost/random.hpp>
#include <boost/program_options.hpp>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include "../src/logic/ELSyntax.h"
#include "../src/inference/MCSat.h"
#include "../src/AllSerializationExports.h"
#include "../test/TestUtilities.h"  // <-- TODO: shouldn't be needed here

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    // build the variable map from our configuration
    po::options_description options("Allowed options");
    options.add_options()
            ("help", "this message")
            ("seed", po::value<unsigned int>(), "rng seed")
            ("name", po::value<std::string>(), "job name (used for file naming)");
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(options).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: volleyball-mcsat [OPTION]..." << std::endl;
        std::cout << options << std::endl;
        return EXIT_FAILURE;
    }
    boost::mt19937 rng;
    if (vm.count("rng")) {
        rng.seed(vm["rng"].as<unsigned int>());
    } else {
        rng.seed((time(NULL) * 32)^getpid());
    }
    // disable debug logging
    FileLog::globalLogLevel() = LOG_ERROR;
    std::string facts("# Video002.mat\n"
            "# BackLeft\n"
            "D-Squat(backleft) @ [1:133]\n"
            "\n"
            "# BackMiddle\n"
            "D-Squat(backmiddle) @ [1:133]\n"
            "D-Squat(backmiddle) @ [156:164]\n"
            "D-Squat(backmiddle) @ [329:352]\n"
            "\n"
            "# BackRight\n"
            "D-Squat(backright) @ [1:31]\n"
            "D-Squat(backright) @ [71:122]\n"
            "D-Squat(backright) @ [303:327]\n"
            "\n"
            "# FrontLeft\n"
            "D-Spike(frontleft) @ [377:393]\n"
            "D-Block(frontleft) @ [214:224]\n"
            "D-Squat(frontleft) @ [1:136]\n"
            "\n"
            "# FrontMiddle\n"
            "D-Spike(frontmiddle) @ [223:238]\n"
            "D-Spike(frontmiddle) @ [409:423]\n"
            "D-Block(frontmiddle) @ [304:319]\n"
            "D-Squat(frontmiddle) @ [23:116]\n"
            "\n"
            "# FrontRight\n"
            "D-Set(frontright) @ [199:209]\n"
            "D-Set(frontright) @ [365:389]\n"
            "D-Squat(frontright) @ [43:117]\n"
            "\n"
            "# RefNet\n"
            "D-Flag(refnet) @ [462:541]\n"
            "\n"
            "# RefBack\n"
            "D-Flag(refback) @ [451:541]\n"
            "\n"
            "# Group\n"
            "D-Huddle() @ [484:541]\n"
            "\n"
            "# Ball\n"
            "D-BallTrajectory() @ [113:153]\n"
            "D-BallTrajectory() @ [159:164]\n"
            "D-BallTrajectory() @ [173:228]\n"
            "D-BallTrajectory() @ [307:312]\n"
            "D-BallTrajectory() @ [322:337]\n"
            "D-BallTrajectory() @ [341:381]\n"
            "D-BallTrajectory() @ [387:392]\n"
            "D-BallTrajectory() @ [421:436]\n"
            "\n"
            "D-BallGoingOut() @ [270:270]\n"
            "D-BallGoingOut() @ [393:393]\n"
            "\n"
            "D-BallGoingin() @ [303:303]\n"
            "D-BallGoingin() @ [412:412]\n"
            "\n"
            "D-BallContact(frontleft) @ [233:233]\n"
            "D-BallContact(frontleft) @ [419:419]\n"
            "\n"
            "D-BallContact(frontright) @ [201:201]\n"
            "D-BallContact(frontright) @ [382:382]\n"
            "\n"
            "D-BallContact(backleft) @ [163:163]\n"
            "D-BallContact(backleft) @ [344:344]\n"
            "\n"
            "D-BallContact(them) @ [108:108]\n"
            "D-BallContact(them) @ [127:127]\n"
            "D-BallContact(them) @ [244:244]\n"
            "D-BallContact(them) @ [260:260]\n"
            "D-BallContact(them) @ [310:310]\n"
            "D-BallContact(them) @ [425:425]\n"
            "D-BallContact(them) @ [441:441]\n"
            "\n"
            );
    std::string formulas("1: [ D-BallContact(backleft) -> BallContact(backleft) ]\n"
            "1: [ D-BallContact(backright) -> BallContact(backright) ]\n"
            "1: [ D-BallContact(backmiddle) -> BallContact(backmiddle) ]\n"
            "1: [ D-BallContact(frontleft) -> BallContact(frontleft) ]\n"
            "1: [ D-BallContact(frontright) -> BallContact(frontright) ]\n"
            "1: [ D-BallContact(frontmiddle) -> BallContact(frontmiddle) ]\n"
            "1: [ BallContact(backleft) -> D-BallContact(backleft) ]\n"
            "1: [ BallContact(backright) -> D-BallContact(backright) ]\n"
            "1: [ BallContact(backmiddle) -> D-BallContact(backmiddle) ]\n"
            "1: [ BallContact(frontleft) -> D-BallContact(frontleft) ]\n"
            "1: [ BallContact(frontright) -> D-BallContact(frontright) ]\n"
            "1: [ BallContact(frontmiddle) -> D-BallContact(frontmiddle) ]\n"
            "1: [ D-Spike(backleft) -> Spike(backleft) ]\n"
            "1: [ D-Spike(backright) -> Spike(backright) ]\n"
            "1: [ D-Spike(backmiddle) -> Spike(backmiddle) ]\n"
            "1: [ D-Spike(frontleft) -> Spike(frontleft) ]\n"
            "1: [ D-Spike(frontright) -> Spike(frontright) ]\n"
            "1: [ D-Spike(frontmiddle) -> Spike(frontmiddle) ]\n"
            "1: [ Spike(backleft) -> D-Spike(backleft) ]\n"
            "1: [ Spike(backright) -> D-Spike(backright) ]\n"
            "1: [ Spike(backmiddle) -> D-Spike(backmiddle) ]\n"
            "1: [ Spike(frontleft) -> D-Spike(frontleft) ]\n"
            "1: [ Spike(frontright) -> D-Spike(frontright) ]\n"
            "1: [ Spike(frontmiddle) -> D-Spike(frontmiddle) ]\n"
            "1: [ D-Set(backleft) -> Set(backleft) ]\n"
            "1: [ D-Set(backright) -> Set(backright) ]\n"
            "1: [ D-Set(backmiddle) -> Set(backmiddle) ]\n"
            "1: [ D-Set(frontleft) -> Set(frontleft) ]\n"
            "1: [ D-Set(frontright) -> Set(frontright) ]\n"
            "1: [ D-Set(frontmiddle) -> Set(frontmiddle) ]\n"
            "1: [ Set(backleft) -> D-Set(backleft) ]\n"
            "1: [ Set(backright) -> D-Set(backright) ]\n"
            "1: [ Set(backmiddle) -> D-Set(backmiddle) ]\n"
            "1: [ Set(frontleft) -> D-Set(frontleft) ]\n"
            "1: [ Set(frontright) -> D-Set(frontright) ]\n"
            "1: [ Set(frontmiddle) -> D-Set(frontmiddle) ]\n"
            "1: [ D-Serve(backleft) -> Serve(backleft) ]\n"
            "1: [ D-Serve(backright) -> Serve(backright) ]\n"
            "1: [ D-Serve(backmiddle) -> Serve(backmiddle) ]\n"
            "1: [ D-Serve(frontleft) -> Serve(frontleft) ]\n"
            "1: [ D-Serve(frontright) -> Serve(frontright) ]\n"
            "1: [ D-Serve(frontmiddle) -> Serve(frontmiddle) ]\n"
            "1: [ Serve(backleft) -> D-Serve(backleft) ]\n"
            "1: [ Serve(backright) -> D-Serve(backright) ]\n"
            "1: [ Serve(backmiddle) -> D-Serve(backmiddle) ]\n"
            "1: [ Serve(frontleft) -> D-Serve(frontleft) ]\n"
            "1: [ Serve(frontright) -> D-Serve(frontright) ]\n"
            "1: [ Serve(frontmiddle) -> D-Serve(frontmiddle) ]\n"
            "1: [ D-Dig(backleft) -> Dig(backleft) ]\n"
            "1: [ D-Dig(backright) -> Dig(backright) ]\n"
            "1: [ D-Dig(backmiddle) -> Dig(backmiddle) ]\n"
            "1: [ D-Dig(frontleft) -> Dig(frontleft) ]\n"
            "1: [ D-Dig(frontright) -> Dig(frontright) ]\n"
            "1: [ D-Dig(frontmiddle) -> Dig(frontmiddle) ]\n"
            "1: [ Dig(backleft) -> D-Dig(backleft) ]\n"
            "1: [ Dig(backright) -> D-Dig(backright) ]\n"
            "1: [ Dig(backmiddle) -> D-Dig(backmiddle) ]\n"
            "1: [ Dig(frontleft) -> D-Dig(frontleft) ]\n"
            "1: [ Dig(frontright) -> D-Dig(frontright) ]\n"
            "1: [ Dig(frontmiddle) -> D-Dig(frontmiddle) ]\n"
            "1: [ D-Block(backleft) -> Block(backleft) ]\n"
            "1: [ D-Block(backright) -> Block(backright) ]\n"
            "1: [ D-Block(backmiddle) -> Block(backmiddle) ]\n"
            "1: [ D-Block(frontleft) -> Block(frontleft) ]\n"
            "1: [ D-Block(frontright) -> Block(frontright) ]\n"
            "1: [ D-Block(frontmiddle) -> Block(frontmiddle) ]\n"
            "1: [ Block(backleft) -> D-Block(backleft) ]\n"
            "1: [ Block(backright) -> D-Block(backright) ]\n"
            "1: [ Block(backmiddle) -> D-Block(backmiddle) ]\n"
            "1: [ Block(frontleft) -> D-Block(frontleft) ]\n"
            "1: [ Block(frontright) -> D-Block(frontright) ]\n"
            "1: [ Block(frontmiddle) -> D-Block(frontmiddle) ]\n"
            "1: [ D-Squat(backleft) -> Squat(backleft) ]\n"
            "1: [ D-Squat(backright) -> Squat(backright) ]\n"
            "1: [ D-Squat(backmiddle) -> Squat(backmiddle) ]\n"
            "1: [ D-Squat(frontleft) -> Squat(frontleft) ]\n"
            "1: [ D-Squat(frontright) -> Squat(frontright) ]\n"
            "1: [ D-Squat(frontmiddle) -> Squat(frontmiddle) ]\n"
            "1: [ Squat(backleft) -> D-Squat(backleft) ]\n"
            "1: [ Squat(backright) -> D-Squat(backright) ]\n"
            "1: [ Squat(backmiddle) -> D-Squat(backmiddle) ]\n"
            "1: [ Squat(frontleft) -> D-Squat(frontleft) ]\n"
            "1: [ Squat(frontright) -> D-Squat(frontright) ]\n"
            "1: [ Squat(frontmiddle) -> D-Squat(frontmiddle) ]\n"
            "1: [ D-Flag(refback) -> Flag(refback) ]\n"
            "1: [ Flag(refback) -> D-Flag(refback) ]\n"
            "1: [ D-Flag(refnet) -> Flag(refnet) ]\n"
            "1: [ Flag(refnet) -> D-Flag(refnet) ]\n"
            "1: [ D-BallContact(them) -> BallContact(them) ]\n"
            "1: [ BallContact(them) -> D-BallContact(them) ]\n"
            "1: [ D-Huddle() -> Huddle() ]\n"
            "1: [ Huddle() -> D-Huddle() ]\n"
            "1: [ D-BallTrajectory() -> BallTrajectory() ]\n"
            "1: [ BallTrajectory() -> D-BallTrajectory() ]\n"
            "1: [ D-BallGoingOut() -> BallGoingOut() ]\n"
            "1: [ BallGoingOut() -> D-BallGoingOut() ]\n"
            "1: [ D-BallGoingIn() -> BallGoingIn() ]\n"
            "1: [ BallGoingIn() -> D-BallGoingIn() ]\n"
            "1: [ D-BallGoingin() -> BallGoingin() ]\n"
            "1: [ BallGoingin() -> D-BallGoingin() ]\n"
            "\n"
            "# a player can have at most one pose at a time\n"
            "10: [ (Spike(backleft) ^ !Set(backleft) ^ !Serve(backleft) ^ !Dig(backleft) ^ !Block(backleft) ^ !Squat(backleft)) v (!Spike(backleft) ^ Set(backleft) ^ !Serve(backleft) ^ !Dig(backleft) ^ !Block(backleft) ^ !Squat(backleft)) v (!Spike(backleft) ^ !Set(backleft) ^ Serve(backleft) ^ !Dig(backleft) ^ !Block(backleft) ^ !Squat(backleft)) v (!Spike(backleft) ^ !Set(backleft) ^ !Serve(backleft) ^ Dig(backleft) ^ !Block(backleft) ^ !Squat(backleft)) v (!Spike(backleft) ^ !Set(backleft) ^ !Serve(backleft) ^ !Dig(backleft) ^ Block(backleft) ^ !Squat(backleft)) v (!Spike(backleft) ^ !Set(backleft) ^ !Serve(backleft) ^ !Dig(backleft) ^ !Block(backleft) ^ Squat(backleft)) v (!Spike(backleft) ^ !Set(backleft) ^ !Serve(backleft) ^ !Dig(backleft) ^ !Block(backleft) ^ !Squat(backleft)) ]\n"
            "10: [ (Spike(backright) ^ !Set(backright) ^ !Serve(backright) ^ !Dig(backright) ^ !Block(backright) ^ !Squat(backright)) v (!Spike(backright) ^ Set(backright) ^ !Serve(backright) ^ !Dig(backright) ^ !Block(backright) ^ !Squat(backright)) v (!Spike(backright) ^ !Set(backright) ^ Serve(backright) ^ !Dig(backright) ^ !Block(backright) ^ !Squat(backright)) v (!Spike(backright) ^ !Set(backright) ^ !Serve(backright) ^ Dig(backright) ^ !Block(backright) ^ !Squat(backright)) v (!Spike(backright) ^ !Set(backright) ^ !Serve(backright) ^ !Dig(backright) ^ Block(backright) ^ !Squat(backright)) v (!Spike(backright) ^ !Set(backright) ^ !Serve(backright) ^ !Dig(backright) ^ !Block(backright) ^ Squat(backright)) v (!Spike(backright) ^ !Set(backright) ^ !Serve(backright) ^ !Dig(backright) ^ !Block(backright) ^ !Squat(backright)) ]\n"
            "10: [ (Spike(backmiddle) ^ !Set(backmiddle) ^ !Serve(backmiddle) ^ !Dig(backmiddle) ^ !Block(backmiddle) ^ !Squat(backmiddle)) v (!Spike(backmiddle) ^ Set(backmiddle) ^ !Serve(backmiddle) ^ !Dig(backmiddle) ^ !Block(backmiddle) ^ !Squat(backmiddle)) v (!Spike(backmiddle) ^ !Set(backmiddle) ^ Serve(backmiddle) ^ !Dig(backmiddle) ^ !Block(backmiddle) ^ !Squat(backmiddle)) v (!Spike(backmiddle) ^ !Set(backmiddle) ^ !Serve(backmiddle) ^ Dig(backmiddle) ^ !Block(backmiddle) ^ !Squat(backmiddle)) v (!Spike(backmiddle) ^ !Set(backmiddle) ^ !Serve(backmiddle) ^ !Dig(backmiddle) ^ Block(backmiddle) ^ !Squat(backmiddle)) v (!Spike(backmiddle) ^ !Set(backmiddle) ^ !Serve(backmiddle) ^ !Dig(backmiddle) ^ !Block(backmiddle) ^ Squat(backmiddle)) v (!Spike(backmiddle) ^ !Set(backmiddle) ^ !Serve(backmiddle) ^ !Dig(backmiddle) ^ !Block(backmiddle) ^ !Squat(backmiddle)) ]\n"
            "10: [ (Spike(frontleft) ^ !Set(frontleft) ^ !Serve(frontleft) ^ !Dig(frontleft) ^ !Block(frontleft) ^ !Squat(frontleft)) v (!Spike(frontleft) ^ Set(frontleft) ^ !Serve(frontleft) ^ !Dig(frontleft) ^ !Block(frontleft) ^ !Squat(frontleft)) v (!Spike(frontleft) ^ !Set(frontleft) ^ Serve(frontleft) ^ !Dig(frontleft) ^ !Block(frontleft) ^ !Squat(frontleft)) v (!Spike(frontleft) ^ !Set(frontleft) ^ !Serve(frontleft) ^ Dig(frontleft) ^ !Block(frontleft) ^ !Squat(frontleft)) v (!Spike(frontleft) ^ !Set(frontleft) ^ !Serve(frontleft) ^ !Dig(frontleft) ^ Block(frontleft) ^ !Squat(frontleft)) v (!Spike(frontleft) ^ !Set(frontleft) ^ !Serve(frontleft) ^ !Dig(frontleft) ^ !Block(frontleft) ^ Squat(frontleft)) v (!Spike(frontleft) ^ !Set(frontleft) ^ !Serve(frontleft) ^ !Dig(frontleft) ^ !Block(frontleft) ^ !Squat(frontleft)) ]\n"
            "10: [ (Spike(frontright) ^ !Set(frontright) ^ !Serve(frontright) ^ !Dig(frontright) ^ !Block(frontright) ^ !Squat(frontright)) v (!Spike(frontright) ^ Set(frontright) ^ !Serve(frontright) ^ !Dig(frontright) ^ !Block(frontright) ^ !Squat(frontright)) v (!Spike(frontright) ^ !Set(frontright) ^ Serve(frontright) ^ !Dig(frontright) ^ !Block(frontright) ^ !Squat(frontright)) v (!Spike(frontright) ^ !Set(frontright) ^ !Serve(frontright) ^ Dig(frontright) ^ !Block(frontright) ^ !Squat(frontright)) v (!Spike(frontright) ^ !Set(frontright) ^ !Serve(frontright) ^ !Dig(frontright) ^ Block(frontright) ^ !Squat(frontright)) v (!Spike(frontright) ^ !Set(frontright) ^ !Serve(frontright) ^ !Dig(frontright) ^ !Block(frontright) ^ Squat(frontright)) v (!Spike(frontright) ^ !Set(frontright) ^ !Serve(frontright) ^ !Dig(frontright) ^ !Block(frontright) ^ !Squat(frontright)) ]\n"
            "10: [ (Spike(frontmiddle) ^ !Set(frontmiddle) ^ !Serve(frontmiddle) ^ !Dig(frontmiddle) ^ !Block(frontmiddle) ^ !Squat(frontmiddle)) v (!Spike(frontmiddle) ^ Set(frontmiddle) ^ !Serve(frontmiddle) ^ !Dig(frontmiddle) ^ !Block(frontmiddle) ^ !Squat(frontmiddle)) v (!Spike(frontmiddle) ^ !Set(frontmiddle) ^ Serve(frontmiddle) ^ !Dig(frontmiddle) ^ !Block(frontmiddle) ^ !Squat(frontmiddle)) v (!Spike(frontmiddle) ^ !Set(frontmiddle) ^ !Serve(frontmiddle) ^ Dig(frontmiddle) ^ !Block(frontmiddle) ^ !Squat(frontmiddle)) v (!Spike(frontmiddle) ^ !Set(frontmiddle) ^ !Serve(frontmiddle) ^ !Dig(frontmiddle) ^ Block(frontmiddle) ^ !Squat(frontmiddle)) v (!Spike(frontmiddle) ^ !Set(frontmiddle) ^ !Serve(frontmiddle) ^ !Dig(frontmiddle) ^ !Block(frontmiddle) ^ Squat(frontmiddle)) v (!Spike(frontmiddle) ^ !Set(frontmiddle) ^ !Serve(frontmiddle) ^ !Dig(frontmiddle) ^ !Block(frontmiddle) ^ !Squat(frontmiddle)) ]\n"
            "\n"
            "# only one player can contact the ball at a time\n"
            "10: [ BallContact(backleft) -> !BallContact(backright) ]\n"
            "10: [ BallContact(backleft) -> !BallContact(backmiddle) ]\n"
            "10: [ BallContact(backleft) -> !BallContact(frontleft) ]\n"
            "10: [ BallContact(backleft) -> !BallContact(frontright) ]\n"
            "10: [ BallContact(backleft) -> !BallContact(frontmiddle) ]\n"
            "10: [ BallContact(backleft) -> !BallContact(them) ]\n"
            "10: [ BallContact(backright) -> !BallContact(backleft) ]\n"
            "10: [ BallContact(backright) -> !BallContact(backmiddle) ]\n"
            "10: [ BallContact(backright) -> !BallContact(frontleft) ]\n"
            "10: [ BallContact(backright) -> !BallContact(frontright) ]\n"
            "10: [ BallContact(backright) -> !BallContact(frontmiddle) ]\n"
            "10: [ BallContact(backright) -> !BallContact(them) ]\n"
            "10: [ BallContact(backmiddle) -> !BallContact(backleft) ]\n"
            "10: [ BallContact(backmiddle) -> !BallContact(backright) ]\n"
            "10: [ BallContact(backmiddle) -> !BallContact(frontleft) ]\n"
            "10: [ BallContact(backmiddle) -> !BallContact(frontright) ]\n"
            "10: [ BallContact(backmiddle) -> !BallContact(frontmiddle) ]\n"
            "10: [ BallContact(backmiddle) -> !BallContact(them) ]\n"
            "10: [ BallContact(frontleft) -> !BallContact(backleft) ]\n"
            "10: [ BallContact(frontleft) -> !BallContact(backright) ]\n"
            "10: [ BallContact(frontleft) -> !BallContact(backmiddle) ]\n"
            "10: [ BallContact(frontleft) -> !BallContact(frontright) ]\n"
            "10: [ BallContact(frontleft) -> !BallContact(frontmiddle) ]\n"
            "10: [ BallContact(frontleft) -> !BallContact(them) ]\n"
            "10: [ BallContact(frontright) -> !BallContact(backleft) ]\n"
            "10: [ BallContact(frontright) -> !BallContact(backright) ]\n"
            "10: [ BallContact(frontright) -> !BallContact(backmiddle) ]\n"
            "10: [ BallContact(frontright) -> !BallContact(frontleft) ]\n"
            "10: [ BallContact(frontright) -> !BallContact(frontmiddle) ]\n"
            "10: [ BallContact(frontright) -> !BallContact(them) ]\n"
            "10: [ BallContact(frontmiddle) -> !BallContact(backleft) ]\n"
            "10: [ BallContact(frontmiddle) -> !BallContact(backright) ]\n"
            "10: [ BallContact(frontmiddle) -> !BallContact(backmiddle) ]\n"
            "10: [ BallContact(frontmiddle) -> !BallContact(frontleft) ]\n"
            "10: [ BallContact(frontmiddle) -> !BallContact(frontright) ]\n"
            "10: [ BallContact(frontmiddle) -> !BallContact(them) ]\n"
            "10: [ BallContact(them) -> !BallContact(backleft) ]\n"
            "10: [ BallContact(them) -> !BallContact(backright) ]\n"
            "10: [ BallContact(them) -> !BallContact(backmiddle) ]\n"
            "10: [ BallContact(them) -> !BallContact(frontleft) ]\n"
            "10: [ BallContact(them) -> !BallContact(frontright) ]\n"
            "10: [ BallContact(them) -> !BallContact(frontmiddle) ]\n"
            "\n"
//            "# if the backleft player served, nobody posed before that action\n"
//            "10: Serve(backleft) -> !(<>{<} [Spike(backleft) v Set(backleft) v Serve(backleft) v Dig(backleft) v Block(backleft) v Squat(backleft)])\n"
//            "10: Serve(backleft) -> !(<>{<} [Spike(backright) v Set(backright) v Serve(backright) v Dig(backright) v Block(backright) v Squat(backright)])\n"
//            "10: Serve(backleft) -> !(<>{<} [Spike(backmiddle) v Set(backmiddle) v Serve(backmiddle) v Dig(backmiddle) v Block(backmiddle) v Squat(backmiddle)])\n"
//            "10: Serve(backleft) -> !(<>{<} [Spike(frontleft) v Set(frontleft) v Serve(frontleft) v Dig(frontleft) v Block(frontleft) v Squat(frontleft)])\n"
//            "10: Serve(backleft) -> !(<>{<} [Spike(frontright) v Set(frontright) v Serve(frontright) v Dig(frontright) v Block(frontright) v Squat(frontright)])\n"
//            "10: Serve(backleft) -> !(<>{<} [Spike(frontmiddle) v Set(frontmiddle) v Serve(frontmiddle) v Dig(frontmiddle) v Block(frontmiddle) v Squat(frontmiddle)])\n"
//            "\n"
//            "# if a huddle occurs, it\'s the final pose\n"
//            "10: Huddle() -> !(<>{>} [Spike(backleft) v Set(backleft) v Serve(backleft) v Dig(backleft) v Block(backleft) v Squat(backleft)])\n"
//            "10: Huddle() -> !(<>{>} [Spike(backright) v Set(backright) v Serve(backright) v Dig(backright) v Block(backright) v Squat(backright)])\n"
//            "10: Huddle() -> !(<>{>} [Spike(backmiddle) v Set(backmiddle) v Serve(backmiddle) v Dig(backmiddle) v Block(backmiddle) v Squat(backmiddle)])\n"
//            "10: Huddle() -> !(<>{>} [Spike(frontleft) v Set(frontleft) v Serve(frontleft) v Dig(frontleft) v Block(frontleft) v Squat(frontleft)])\n"
//            "10: Huddle() -> !(<>{>} [Spike(frontright) v Set(frontright) v Serve(frontright) v Dig(frontright) v Block(frontright) v Squat(frontright)])\n"
//            "10: Huddle() -> !(<>{>} [Spike(frontmiddle) v Set(frontmiddle) v Serve(frontmiddle) v Dig(frontmiddle) v Block(frontmiddle) v Squat(frontmiddle)])\n"
//            "\n"
            "# if a player contacts the ball, they are in some sort of pose\n"
            "10: [ BallContact(backleft) -> Spike(backleft) v Set(backleft) v Serve(backleft) v Dig(backleft) v Block(backleft) v Squat(backleft) ]\n"
            "10: [ BallContact(backright) -> Spike(backright) v Set(backright) v Serve(backright) v Dig(backright) v Block(backright) v Squat(backright) ]\n"
            "10: [ BallContact(backmiddle) -> Spike(backmiddle) v Set(backmiddle) v Serve(backmiddle) v Dig(backmiddle) v Block(backmiddle) v Squat(backmiddle) ]\n"
            "10: [ BallContact(frontleft) -> Spike(frontleft) v Set(frontleft) v Serve(frontleft) v Dig(frontleft) v Block(frontleft) v Squat(frontleft) ]\n"
            "10: [ BallContact(frontright) -> Spike(frontright) v Set(frontright) v Serve(frontright) v Dig(frontright) v Block(frontright) v Squat(frontright) ]\n"
            "10: [ BallContact(frontmiddle) -> Spike(frontmiddle) v Set(frontmiddle) v Serve(frontmiddle) v Dig(frontmiddle) v Block(frontmiddle) v Squat(frontmiddle) ]\n"
            "\n"
//            "# if a player served, nobody on our team will touch the ball before the opponents\n"
//            "10: Serve(backleft) ^{<} BallContact(them) -> (<>{d} BallContact(them)) v (!<>{d} [BallContact(backright) v BallContact(backmiddle) v BallContact(frontleft) v BallContact(frontright) v BallContact(frontmiddle)])\n"
//            "\n"
            "# only the back left player may serve\n"
            "10: [ !Serve(backright) ^ !Serve(backmiddle) ^ !Serve(frontleft) ^ !Serve(frontright) ^ !Serve(frontmiddle) ]\n"
            "\n"
            "# only players in the front row may spike\n"
            "10: [ !Spike(backleft) ^ !Spike(backright) ^ !Spike(backmiddle) ]\n"
            "\n"
            );

    Domain d = loadDomainWithStreams(facts, formulas);

    std::set<unsigned int> timePoints;
    for (Domain::fact_const_iterator it = d.facts_begin(); it != d.facts_end(); it++) {
        SISet set = it->second;
        for (SISet::const_iterator siIt = set.begin(); siIt != set.end(); siIt++) {
            SpanInterval si = *siIt;
            timePoints.insert(si.start().start());
            timePoints.insert(si.start().finish());
            timePoints.insert(si.finish().start());
            timePoints.insert(si.finish().finish());
        }
    }

    std::cout << "found " << timePoints.size() << " timepoints." << std::endl;

    MCSat mcSatSolver(&d);
    mcSatSolver.setBurnInIterations(1);
    mcSatSolver.setNumSamples(1);
    std::cout << "running mcSatSolver with " << mcSatSolver.burnInIterations() << " burn in iterations and a sample size of " << mcSatSolver.numSamples() << std::endl;
    std::string prefix = (vm.count("name") ? vm["name"].as<std::string>() : "mcsat-volleyball");
    std::cout << "saving random seed..." << std::endl;
    {
        std::string rngFilename = prefix + "-seed.txt";
        std::ofstream rngOut(rngFilename.c_str(), std::ios_base::trunc);
        if (!rngOut) {
            std::cerr << "unable to open file " + rngFilename + "for RNG writing." << std::endl;
        } else {
            rngOut << rng;
        }
    }
    //std::cout << "rng = " << rng << std::endl;
    std::cout << "starting run..." << std::endl;
    mcSatSolver.setSampleStrategy(new MCSatSampleLiquidlyStrategy());
    std::clock_t start = std::clock();
    mcSatSolver.run(rng);
    std::clock_t end = std::clock();
    std::cout << "ran in " << (end - start)/CLOCKS_PER_SEC << " seconds." << std::endl;
    std::cout << "saving model file..." << std::endl;
    {
        std::string outFilename = prefix + "-model.dat";
        std::ofstream outFile(outFilename.c_str(), std::ios_base::trunc);
        if (!outFile) {
            std::cerr << "unable to open file " + outFilename + " for model writing." << std::endl;
        } else {
            boost::archive::text_oarchive tout(outFile);
            registerAllPELTypes(tout);
            tout << mcSatSolver;
        }
    }
}
