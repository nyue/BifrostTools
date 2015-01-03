#include <bifrostapi/bifrost_om.h>
#include <bifrostapi/bifrost_stateserver.h>
#include <bifrostapi/bifrost_component.h>
#include <bifrostapi/bifrost_fileio.h>
#include <bifrostapi/bifrost_fileutils.h>
#include <bifrostapi/bifrost_string.h>
#include <bifrostapi/bifrost_stringarray.h>
#include <bifrostapi/bifrost_refarray.h>
#include <bifrostapi/bifrost_channel.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <stdexcept>

namespace po = boost::program_options;

// A helper function to simplify the main part.
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    return os;
}

void process_bifrost_file(const std::string& bifrost_filename);

int main(int argc, char **argv)
{

    try {
        typedef std::vector<std::string> StringContainer;
        std::string bifrost_filename;
        po::options_description desc("Allowed options");
        desc.add_options()
            ("version", "print version string")
            ("help", "produce help message")
            ("input-file", po::value<std::vector<std::string> >(),
             "input files")
            ;

        po::positional_options_description p;
        p.add("input-file", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
//        if (vm.count("config")) {
//            std::cout << "Configuration file is " <<
//                vm["config"].as<std::string>().c_str() << "\n";
//            std::cout << "config_file variable " << config_file.c_str() << std::endl;
//        }
//        if (vm.count("compression")) {
//            std::cout << "Compression value is " <<
//                vm["compression"].as<int>() << "\n";
//            std::cout << "compression variable " << compression << std::endl;
//        }
        if (vm.count("input-file"))
        {
            std::cout << "Input files are: "
                      << vm["input-file"].as< std::vector<std::string> >() << "\n";
            if (vm["input-file"].as< std::vector<std::string> >().size() == 1)
            {
                bifrost_filename = vm["input-file"].as< std::vector<std::string> >()[0];
            }
            else
            {
                std::cout << desc << "\n";
                return 1;
            }
        }

        if (bifrost_filename.size()>0)
            process_bifrost_file(bifrost_filename);
        else
        {
            std::cout << desc << "\n";
            return 1;
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

void process_bifrost_file(const std::string& bifrost_filename)
{
    Bifrost::API::String biffile = bifrost_filename.c_str();
    Bifrost::API::ObjectModel om;
    Bifrost::API::FileIO fileio = om.createFileIO( biffile );
    const Bifrost::API::BIF::FileInfo& info = fileio.info();

    std::cout << boost::format("Version        : %1%") % info.version << std::endl;
    std::cout << boost::format("Frame          : %1%") % info.frame << std::endl;
    std::cout << boost::format("Channel count  : %1%") % info.channelCount << std::endl;
    std::cout << boost::format("Component name : %1%") % info.componentName.c_str() << std::endl;
    std::cout << boost::format("Component type : %1%") % info.componentType << std::endl;
    std::cout << boost::format("Object name    : %1%") % info.objectName.c_str() << std::endl;
    std::cout << boost::format("Layout name    : %1%") % info.layoutName.c_str() << std::endl;

    for (size_t channelIndex=0;channelIndex<info.channelCount;channelIndex++)
    {
        std::cout << std::endl;
        const Bifrost::API::BIF::FileInfo::ChannelInfo& channelInfo = fileio.channelInfo(channelIndex);
        std::cout << boost::format("        Channel name  : %1%") % channelInfo.name.c_str() << std::endl;
        std::cout << boost::format("        Data type     : %1%") % channelInfo.dataType << std::endl;
        std::cout << boost::format("        Max depth     : %1%") % channelInfo.maxDepth << std::endl;
        std::cout << boost::format("        Tile count    : %1%") % channelInfo.tileCount << std::endl;
        std::cout << boost::format("        Element count : %1%") % channelInfo.elementCount << std::endl;
    }
}
