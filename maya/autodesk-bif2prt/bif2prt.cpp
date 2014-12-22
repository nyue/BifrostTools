#include "bif2prt.h"
#include <algorithm>

#include <bifrostapi/bifrost_om.h>
#include <bifrostapi/bifrost_stateserver.h>
#include <bifrostapi/bifrost_component.h>
#include <bifrostapi/bifrost_fileio.h>
#include <bifrostapi/bifrost_fileutils.h>
#include <bifrostapi/bifrost_string.h>
#include <bifrostapi/bifrost_stringarray.h>
#include <bifrostapi/bifrost_refarray.h>

namespace {
void usage(char **argv)
{
    std::cerr << "Usage:" << std::endl;
	std::cerr << "   bif2prt.exe " << "[-den -pos -vel -vor]" << "-f file.bif " << "[-o file.prt]" << std::endl;
	std::cerr << "   -den: density channel. " << std::endl;	
	std::cerr << "   -pos: position channel. " << std::endl;	
	std::cerr << "   -vel: velocity channel. " << std::endl;	
	std::cerr << "   -vor: vorticity channel. " << std::endl;	
	std::cerr << "   note: if no channel options are specified, all channels in the BIF file will be considered." << std::endl;	
	std::cerr << std::endl;
	std::cerr << "   -f file.bif: mandatory BIF file to load." << std::endl;
	std::cerr << "   -o file.prt: optional .prt file to generate. If omitted, the BIF file name is used as the .prt file name." << std::endl;
	std::cerr << std::endl;
	std::cerr << "   e.g. bif2prt.exe -pos -vel -vor -f myfile.bif" << std::endl;
}

char* getOption( char** begin, char** end, const std::string & option )
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

}

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 9 ) {
		usage( argv );
		exit(1);
	}

	Bifrost::API::StringArray optionNames;

	char* option = getOption( argv, argv+argc, "-den" );	
	if (option) {
		optionNames.add( "density" );
	}

	option = getOption( argv, argv+argc, "-pos" );	
	if (option) {
		optionNames.add( "position" );
	}

	option = getOption( argv, argv+argc, "-vel" );	
	if (option) {
		optionNames.add( "velocity" );
	}

	option = getOption( argv, argv+argc, "-vor" );	
	if (option) {
		optionNames.add( "vorticity" );
	}

	option = getOption( argv, argv+argc, "-f" );
	Bifrost::API::String biffile;
	size_t pos;
	if (option) {
		biffile = Bifrost::API::File::forwardSlashes( option );
		pos = biffile.rfind(".bif");
		if (pos==Bifrost::API::String::npos) {
			usage( argv );
			exit(1);
		}
	}

	option = getOption( argv, argv+argc, "-o" );
	Bifrost::API::String prtfile;
	if (option) {
		prtfile = Bifrost::API::File::forwardSlashes( option );
	}
	else {
		prtfile = biffile.substr( 0 , pos );
		prtfile += ".prt";
	}

	Bifrost::API::ObjectModel om;
	Bifrost::API::FileIO fileio = om.createFileIO( biffile );
	Bifrost::API::StateServer ss = fileio.load( );

	if ( !ss.valid() ) {
		std::cerr << "bif2prt : file loading error" << std::endl;
		exit(1);
	}

	Bifrost::API::Component component = ss.components()[0];
	if ( component.type() != Bifrost::API::PointComponentType ) {
		std::cerr << "bif2prt : wrong component (" << component.type() << ")" << std::endl;
		exit(1);
	}

	// select channels as specified on input
	PRTConverter::ChannelPairNames namePairs;
	Bifrost::API::RefArray channels = component.channels();
	for (size_t i=0; i<optionNames.count(); i++ ) {
		Bifrost::API::String optionName = optionNames[i];
		for (size_t j=0; j<channels.count(); j++ ) {
			Bifrost::API::String chname = Bifrost::API::Base(channels[j]).name();
			if ( chname.find( optionName ) != Bifrost::API::String::npos ) {
				if ( optionName == "position" ) {
					namePairs.push_back( PRTConverter::Pair(chname,"Position") );
					break;
				}
				else if ( optionName == "velocity" ) {
					namePairs.push_back( PRTConverter::Pair(chname,"Velocity") );
					break;
				}
				else if ( optionName == "vorticity" ) {
					namePairs.push_back( PRTConverter::Pair(chname,"Vorticity") );
					break;
				}
				else if ( optionName == "density" ) {
					namePairs.push_back( PRTConverter::Pair(chname,"Density") );
					break;
				}
			}
		}
	}

	// get all the component channels by default
	if ( optionNames.count() == 0 ) {
		Bifrost::API::RefArray channels = component.channels();
		for (size_t i=0; i<channels.count(); i++ ) {
			Bifrost::API::String chname = Bifrost::API::Base(channels[i]).name();
			// Use PRT naming convention
			if ( chname.find( "position" ) != Bifrost::API::String::npos ) {
				namePairs.push_back( PRTConverter::Pair(chname,"Position") );
			}
			else if ( chname.find( "velocity" ) != Bifrost::API::String::npos ) {
				namePairs.push_back( PRTConverter::Pair(chname,"Velocity") );
			}
			else if ( chname.find( "vorticity" ) != Bifrost::API::String::npos ) {
				namePairs.push_back( PRTConverter::Pair(chname,"Vorticity") );
			}
			else if ( chname.find( "density" ) != Bifrost::API::String::npos ) {
				namePairs.push_back( PRTConverter::Pair(chname,"Density") );
			}
			else {
				// for other channels we use the last token in the channel name as the PRT name
				Bifrost::API::StringArray splitName = chname.split("/");
				Bifrost::API::String validPRTName;
				if ( splitName.count() == 1 ) {
					// there is no / separator
					validPRTName = splitName[0]; 
				}
				else {
					validPRTName = splitName[1];
				}
				namePairs.push_back( PRTConverter::Pair(chname,validPRTName.data()) );
			}
		}
	}
	
	PRTConverter prt;
	bool result = prt.write( prtfile.data(), component, namePairs );
	if (!result) {
		std::cerr << "bif2prt: Conversion failed" << std::endl;
		exit(1);
	}
	else {
		std::cerr << "bif2prt: Conversion succeeded!" << std::endl;
		std::cerr << "File created: " << Bifrost::API::File::backwardSlashes(prtfile).data() << std::endl;
	}

	return 0;
}