#include <iostream>
#include <vector>
#include <string>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include <houdini_utils.h>

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

		Bifrost2HoudiniGeo b2hg(bifrost_filename,bgeo_filename);

		b2hg.process();
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
// == Emacs ================
// -------------------------
// Local variables:
// tab-width: 4
// indent-tabs-mode: t
// c-basic-offset: 4
// end:
//
// == vi ===================
// -------------------------
// Format block
// ex:ts=4:sw=4:expandtab
// -------------------------
