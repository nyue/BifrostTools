#include "Bifrost2HoudiniGeo.h"
#include <boost/format.hpp>

// Houdini header - START
#include <GU/GU_Detail.h>
#include <UT/UT_FileUtil.h>
#include <UT/UT_Options.h>
#include <SYS/SYS_Version.h>

#if SYS_VERSION_MAJOR_INT >= 15
#include <GA/GA_SaveOptions.h>
#endif

// Houdini header - END

// Bifrost headers - START
#include <BifrostHeaders.h>
// Bifrost headers - END

Bifrost2HoudiniGeo::Bifrost2HoudiniGeo(const std::string& i_bifrost_filename,const std::string& i_hougeo_filename)
: _bifrost_filename(i_bifrost_filename)
, _hougeo_filename(i_hougeo_filename)
{

}

Bifrost2HoudiniGeo::~Bifrost2HoudiniGeo()
{

}

bool Bifrost2HoudiniGeo::process()
{
	// Bifrost file handling
	Bifrost::API::String biffile = _bifrost_filename.c_str();
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
		std::cerr << boost::format("Unable to load the content of the Bifrost file \"%1%\"") % _bifrost_filename.c_str()
				  << std::endl;
		return false;
	}

	// Houdini Geometry handling
	GU_Detail gdp;
	UT_BoundingBox  bounds;

	/* Chat with Igor Zanic indicates that simple points with attributes
	 * is sufficient, no need to create particle system
	 */
	UT_Options	options("bool   geo:saveinfo",	(int)1,
						"string info:software", "bif2bgeo",
						"string info:comment", "info@proceduralinsight.com",
						NULL);
#if SYS_VERSION_MAJOR_INT >= 15
	GA_SaveOptions gaso;
	gdp.save(_hougeo_filename.c_str(),&gaso);
#else
	gdp.save(_hougeo_filename.c_str(),&options);
#endif
	return true;
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
