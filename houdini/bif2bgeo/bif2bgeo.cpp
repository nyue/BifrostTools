#include <iostream>
#include <vector>
#include <string>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

// Houdini header - START
#include <GU/GU_Detail.h>
#include <UT/UT_FileUtil.h>
// Houdini header - END

// Bifrost headers - START
#include <BifrostHeaders.h>
// Bifrost headers - END

namespace po = boost::program_options;

int main(int argc, char **argv)
{
	try {
    std::string density_channel_name("density");
    std::string position_channel_name("position");
    std::string velocity_channel_name("velocity");
    std::string vorticity_channel_name("vorticity");
    std::string droplet_channel_name("droplet");
    std::string bifrost_filename;
    std::string bgeo_filename;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Produce help message")
        ("density", po::value<std::string>(&density_channel_name)->default_value(density_channel_name),
         (boost::format("Density channel name. Defaults to '%1%'") % density_channel_name).str().c_str())
		("position", po::value<std::string>(&position_channel_name)->default_value(position_channel_name),
	     (boost::format("Position channel name. Defaults to '%1%'") % position_channel_name).str().c_str())
		("velocity", po::value<std::string>(&velocity_channel_name)->default_value(velocity_channel_name),
	     (boost::format("Velocity channel name. Defaults to '%1%'") % velocity_channel_name).str().c_str())
		("vorticity", po::value<std::string>(&vorticity_channel_name)->default_value(vorticity_channel_name),
	     (boost::format("Vorticity channel name. Defaults to '%1%'") % vorticity_channel_name).str().c_str())
		("droplet", po::value<std::string>(&droplet_channel_name)->default_value(droplet_channel_name),
	     (boost::format("Droplet channel name. Defaults to '%1%'") % droplet_channel_name).str().c_str())
        ("bif", po::value<std::string>(&bifrost_filename),
         "Bifrost file. [Required]")
        ("geo", po::value<std::string>(&bgeo_filename),
         "(B)geo file. [Required]")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") || bifrost_filename.empty() || bgeo_filename.empty()) {
        std::cout << desc << "\n";
        return 1;
    }

    // Bifrost file handling
    Bifrost::API::String biffile = bifrost_filename.c_str();
    Bifrost::API::ObjectModel om;
    Bifrost::API::FileIO fileio = om.createFileIO( biffile );
    const Bifrost::API::BIF::FileInfo& info = fileio.info();
    bool is_bifrost_liquid_file(true);

    // Need to determine is the BIF file contains Foam or Liquid particle info
    if (std::string(info.componentName.c_str()).find("Foam")!=std::string::npos)
    	is_bifrost_liquid_file = false;

    // Need to load the entire file's content to process
    Bifrost::API::StateServer ss = fileio.load( );
    if (ss.valid())
    {
        size_t numComponents = ss.components().count();
        if (numComponents>0)
        {
            std::cout << boost::format("numComponents = %1%") % numComponents << std::endl;
        }
    }
    else
    {
        std::cerr << boost::format("Unable to load the content of the Bifrost file \"%1%\"") % bifrost_filename.c_str()
                  << std::endl;
        return 1;
    }

    // Houdini Geometry handling
    GU_Detail gdp;
    UT_BoundingBox  bounds;
    UT_Options	options("bool   geo:saveinfo",	(int)1,
    					"string info:software", "bif2bgeo",
						"string info:comment", "info@proceduralinsight.com",
						NULL);
    gdp.save(bgeo_filename.c_str(),&options);
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
