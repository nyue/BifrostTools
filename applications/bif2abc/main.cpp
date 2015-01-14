#include <stdlib.h>
#include <utils/BifrostUtils.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <strstream>
#include <stdexcept>
#include <OpenEXR/ImathBox.h>

// Alembic headers - START
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/Util/All.h>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
// Alembic headers - END

// Bifrost headers - START
#include <bifrostapi/bifrost_om.h>
#include <bifrostapi/bifrost_stateserver.h>
#include <bifrostapi/bifrost_component.h>
#include <bifrostapi/bifrost_fileio.h>
#include <bifrostapi/bifrost_fileutils.h>
#include <bifrostapi/bifrost_string.h>
#include <bifrostapi/bifrost_stringarray.h>
#include <bifrostapi/bifrost_refarray.h>
#include <bifrostapi/bifrost_channel.h>
// Bifrost headers - END

namespace po = boost::program_options;

// A helper function to simplify the main part.
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    return os;
}

bool process_bifrost_file(const std::string& bifrost_filename)
{
    return true;
}

int main(int argc, char **argv)
{

    try {
        typedef std::vector<std::string> StringContainer;
        std::string position_channel_name("position");
        std::string velocity_channel_name("velocity");
        StringContainer bifrost_filename_collection;
        std::string bifrost_filename;
        float fps = 24.0f;
        po::options_description desc("Allowed options");
        desc.add_options()
            ("version", "print version string")
            ("help", "produce help message")
            ("fps", po::value<float>(&fps),
                    "Frames per second to scale velocity when determining the velocity-attenuated bounding box. Defaults to 24.0")
            ("input-files", po::value<StringContainer>(&bifrost_filename_collection),
             "input files")
            ;

        po::positional_options_description p;
        p.add("input-files", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
        if (vm.count("input-file"))
        {
            std::cout << "Input files are: "
                      << vm["input-file"].as< StringContainer >() << "\n";
            if (vm["input-file"].as< StringContainer >().size() == 1)
            {
                bifrost_filename = vm["input-file"].as< StringContainer >()[0];
            }
            else
            {
                std::cout << desc << "\n";
                return 1;
            }
        }
        std::cout << "fps = " << fps << std::endl;
        StringContainer::const_iterator iter = bifrost_filename_collection.begin();
        StringContainer::const_iterator eIter = bifrost_filename_collection.end();
        for (;iter!=eIter;++iter)
        {
            std::cout << "bifrost file to process " << iter->c_str() << std::endl;
            process_bifrost_file(*iter);
//            process_bifrost_file(bifrost_filename,
//                                 position_channel_name,
//                                 velocity_channel_name,
//                                 bbox_type,
//                                 bbox_type==BBOX::PointsWithVelocity?&fps:0);
        }
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
