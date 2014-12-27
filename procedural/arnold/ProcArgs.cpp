#include "ProcArgs.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

ProcArgs::ProcArgs()
: proceduralNode(0)
, pointRadius(0.01f)
, enableVelocityMotionBlur(false)
, performEmission(false)
, bifrostTileIndex(0)
, bifrostTileDepth(0)
{
}

int ProcArgs::processDataStringAsArgcArgv(int argc, const char **argv)
{
    try {
        float radius = 0.01f; // default - renders point of size 0.01
        std::string bifrost_filename;
        size_t tileIndex = 0;
        size_t tileDepth = 0;
        po::options_description desc("Allowed options");
        desc.add_options()
            ("version", "print version string")
            ("help", "produce help message")
            ("radius", po::value<float>(&radius),
             "radius for RIB point geometry.")
            ("velocity-blur", "use velocity for motion blur.")
             ("bif", po::value<std::string>(&bifrost_filename),
              "bifrost filename.")
              ("tile-index", po::value<size_t>(&tileIndex),
               "bifrost tile index.")
               ("tile-depth", po::value<size_t>(&tileDepth),
                "bifrost tile depth.")
             ("emit", "non-root level, perform emission.")
            ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
        if (vm.count("velocity-blur")) {
            enableVelocityMotionBlur = true;
        }
        if (vm.count("emit")) {
            performEmission = true;
        }
        pointRadius = radius;
        bifrostFilename = bifrost_filename;
        std::cout << "XXXXXXXXXXXXXX bifrost_filename : " << bifrost_filename << std::endl;
        bifrostTileIndex = tileIndex;
        bifrostTileDepth = tileDepth;
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!\n";
    }
    return 0;
}
