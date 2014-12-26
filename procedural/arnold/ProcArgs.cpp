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
        bool velocity_blur = false; // default - no velocity motion blur
        bool emit_geometry = false; // default - do not emit geometry
        std::string bifrost_filename;
        size_t tileIndex = 0;
        size_t tileDepth = 0;
        po::options_description desc("Allowed options");
        desc.add_options()
            ("version", "print version string")
            ("help", "produce help message")
            ("radius", po::value<float>(&radius),
             "radius for RIB point geometry.")
            ("velocity_blur", po::value<bool>(&velocity_blur),
             "use velocity for motion blur.")
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
        pointRadius = radius;
        enableVelocityMotionBlur = velocity_blur;
        performEmission = emit_geometry;
        bifrostFilename = bifrost_filename;
        bifrostTileIndex = tileIndex;
        bifrostTileDepth = tileDepth;

        /*
        if (vm.count("radius")) {
            std::cout << "Radius value is " <<
                vm["radius"].as<float>() << "\n";
            std::cout << "radius variable " << radius << std::endl;
        }
        if (vm.count("velocity_blur")) {
            std::cout << "Velocity blur value is " <<
                vm["velocity_blur"].as<int>() << "\n";
            std::cout << "velocity_blur variable " << velocity_blur << std::endl;
        }
        */
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
