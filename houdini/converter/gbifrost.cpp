#include <iostream>
#include <vector>
#include <string>
#include <boost/format.hpp>

#include <houdini_utils.h>

int main(int argc, char** argv)
{
	if (argc!=3)
	{
		fprintf(stderr,"Usage : gbifrost inFile outFile\n");
		return 1;
	}
    std::string bifrost_filename(argv[1]);
    std::string bgeo_filename(argv[2]);

	Bifrost2HoudiniGeo b2hg(bifrost_filename,bgeo_filename);

	if (!b2hg.process())
	{
		std::cerr << boost::format("gbifrost : Failed to process %1% or write the output %2%") % bifrost_filename % bgeo_filename << std::endl;
		return 1;
	}
    return 0;
}
