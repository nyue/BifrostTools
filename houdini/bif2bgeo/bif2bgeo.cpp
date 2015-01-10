#include <utils/BifrostUtils.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <strstream>
#include <stdexcept>
#include <OpenEXR/ImathBox.h>

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

// Houdini headers - START
#include <GU/GU_Detail.h>
// Houdini headers - END

namespace po = boost::program_options;

int process_points(const Bifrost::API::Component& component,
                   const std::string& position_channel_name,
                   const std::string& velocity_channel_name,
				   GU_Detail&       gdp)
{
    int positionChannelIndex = findChannelIndexViaName(component,position_channel_name.c_str());
    if (positionChannelIndex<0)
        return 1;
    int velocityChannelIndex = findChannelIndexViaName(component,velocity_channel_name.c_str());
    if (velocityChannelIndex<0)
        return 1;
    const Bifrost::API::Channel& position_ch = component.channels()[positionChannelIndex];
    if (!position_ch.valid())
        return 1;
    const Bifrost::API::Channel& velocity_ch = component.channels()[velocityChannelIndex];
    if (!velocity_ch.valid())
        return 1;
    if ( position_ch.dataType() != Bifrost::API::FloatV3Type)
        return 1;
    if ( velocity_ch.dataType() != Bifrost::API::FloatV3Type)
        return 1;

    Bifrost::API::Layout layout = component.layout();
    size_t depthCount = layout.depthCount();
    for ( size_t d=0; d<depthCount; d++ )
    {
        for ( size_t t=0; t<layout.tileCount(d); t++ )
        {
            Bifrost::API::TreeIndex tindex(t,d);
            if ( !position_ch.elementCount( tindex ) )
            {
                // nothing there
                continue;
            }
            if (position_ch.elementCount( tindex ) != velocity_ch.elementCount( tindex ))
                return 1;
            const Bifrost::API::TileData<amino::Math::vec3f>& position_tile_data = position_ch.tileData<amino::Math::vec3f>( tindex );
            const Bifrost::API::TileData<amino::Math::vec3f>& velocity_tile_data = velocity_ch.tileData<amino::Math::vec3f>( tindex );
            bool process_velocities = position_tile_data.count() == velocity_tile_data.count();
            GA_RWAttributeRef   v_attrib;
            if (process_velocities)
            {
            	v_attrib = gdp.addFloatTuple(GA_ATTRIB_POINT, "v", 3);
            	if (v_attrib.isValid())
            		v_attrib.getAttribute()->setTypeInfo(GA_TYPE_VECTOR);
            	else
            		std::cerr << "v_attrib is NOT valid" << std::endl;
            }

            for (size_t i=0; i<position_tile_data.count(); i++ )
            {
                UT_Vector4 point_position(position_tile_data[i][0],
                			 position_tile_data[i][1],
							 position_tile_data[i][2]);
                GEO_Point *ppt = gdp.appendPointElement();
                ppt->setPos(point_position);
                if (process_velocities)
                	ppt->setValue<UT_Vector3>(v_attrib,UT_Vector3(velocity_tile_data[i][0],
                			velocity_tile_data[i][1],
							velocity_tile_data[i][2]));
            }
        }
    }
    
    return 0;
}

int process_bifrost_file(const std::string& bifrost_filename,
                         const std::string& position_channel_name,
                         const std::string& velocity_channel_name,
						 GU_Detail&       gdp)
{
    Bifrost::API::String biffile = bifrost_filename.c_str();
    Bifrost::API::ObjectModel om;
    Bifrost::API::FileIO fileio = om.createFileIO( biffile );
    const Bifrost::API::BIF::FileInfo& info = fileio.info();

    // Need to load the entire file's content to process
    Bifrost::API::StateServer ss = fileio.load( );
    if (ss.valid())
    {
        size_t numComponents = ss.components().count();
        for (size_t componentIndex=0;componentIndex<numComponents;componentIndex++)
        {
            Bifrost::API::Component component = ss.components()[componentIndex];
            Bifrost::API::TypeID componentType = component.type();
            if (componentType == Bifrost::API::PointComponentType)
            {
                process_points(component,position_channel_name,velocity_channel_name,gdp);
            }
            else if (componentType == Bifrost::API::VoxelComponentType)
            {
                // process_voxels(component,gdp);
            }
        }
    }
    else
    {
        std::cerr << boost::format("Unable to load the content of the Bifrost file \"%1%\"") % bifrost_filename.c_str()
                  << std::endl;
        return 1;
    }
    
    return 0;
}

#ifdef HOUDINI_CODE
#include <GU/GU_Detail.h>
#include <iostream>

int
main(int argc, char *argv[])
{
    GU_Detail       gdp;
    UT_BoundingBox  bounds;
    double scale = 10.0;
	
    // Evaluate the iso-surface inside this bounding box                                                                                                                                                         
    bounds.setBounds(-scale, -scale, -scale, scale, scale, scale);

    // Add velocity attribute
    GA_RWAttributeRef   v_attrib = gdp.addFloatTuple(GA_ATTRIB_POINT, "v", 3);
    if (v_attrib.isValid())
    	v_attrib.getAttribute()->setTypeInfo(GA_TYPE_VECTOR);
    else
    	std::cerr << "v_attrib is NOT valid" << std::endl;

	// Create particle points
    const int numParticles = 100000;
    for (size_t i = 0; i<numParticles;i++)
    {
        UT_Vector4 v(
                     (drand48()-0.5)*scale,
                     (drand48()-0.5)*scale,
                     (drand48()-0.5)*scale
                     );
        GEO_Point *ppt = gdp.appendPointElement();
        ppt->setPos(v);
        ppt->setValue<UT_Vector3>(v_attrib,UT_Vector3(0,1,0));
    }

    // Save to sphere.bgeo                                                                                                                                                                                       
    gdp.save("points.bgeo", NULL);
	
    return 0;
}
#endif // HOUDINI_CODE

int main(int argc, char **argv)
{

    try {
        std::string position_channel_name("position");
        std::string velocity_channel_name("velocity");
        std::string bifrost_filename;
        std::string bgeo_filename;
        po::options_description desc("Allowed options");
        desc.add_options()
        		("version", "print version string")
				("help", "produce help message")
				("input-file,i", po::value<std::string >(&bifrost_filename),
						"Bifrost file")
				("output-file,o", po::value<std::string >(&bgeo_filename),
						"Houdini BGEO file");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);


        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
        if (bifrost_filename.size()>4 && bgeo_filename.size()>4)
        {
        	// Do conversion
            GU_Detail       gdp;

            process_bifrost_file(bifrost_filename,
                                 position_channel_name,
                                 velocity_channel_name,
								 gdp);

            gdp.save(bgeo_filename.c_str(),NULL);
        }
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
