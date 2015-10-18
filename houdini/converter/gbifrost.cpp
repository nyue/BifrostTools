#include <iostream>
#include <vector>
#include <string>
#include <boost/format.hpp>

// Houdini header - START
#include <GU/GU_Detail.h>
#include <UT/UT_FileUtil.h>
// Houdini header - END

// Bifrost headers - START
#include <BifrostHeaders.h>
// Bifrost headers - END

int main(int argc, char** argv)
{
	if (argc!=3)
	{
		fprintf(stderr,"Usage : gbifrost inFile outFile\n");
		return 1;
	}
    std::string bifrost_filename(argv[1]);
    std::string bgeo_filename(argv[2]);

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

    return 0;
}
