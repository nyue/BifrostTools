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

#include <BifrostHeaders.h>

namespace po = boost::program_options;

// A helper function to simplify the main part.
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    return os;
}

Alembic::AbcGeom::OXform
addXform(Alembic::Abc::OObject parent, std::string name)
{
    Alembic::AbcGeom::OXform xform(parent, name.c_str());

    return xform;
}

CANNOT BUILD NEED TO CONSIDER THROWING EXCEPTION

/*
http://stackoverflow.com/questions/2639255/c-return-a-null-object-if-search-result-not-found

boost::optional<Attr&> getAttribute(const string& attribute_name) const
{
   //search collection
   //if found at i
        return attributes[i];
   //if not found
        return boost::optional<Attr&>();
}
 */
#ifdef OLD_RETRIEVAL
typedef boost::optional<const Bifrost::API::Channel&> OptionalChannelRef;

OptionalChannelRef
get_channel(const Bifrost::API::Component& i_component,
				 const std::string& i_channel_name,
				 const Bifrost::API::DataType& i_expected_type)
{
    int channelIndex = findChannelIndexViaName(i_component,i_channel_name.c_str());
    if (channelIndex<0)
    {
        std::cerr << boost::format("get_channel() : channel '%1%' not found") % i_channel_name << std::endl;
        return OptionalChannelRef();
    }
    const Bifrost::API::Channel& channel = i_component.channels()[channelIndex];
    if (!channel.valid())
    {
        std::cerr << boost::format("get_channel() : channel '%1%' not valid") % i_channel_name << std::endl;
        return OptionalChannelRef();
    }
    if ( channel.dataType() != i_expected_type)
    {
        std::cerr << boost::format("get_channel() : channel '%1%' of type %2% is different from expected type %3%") % i_channel_name % channel.dataType() % i_expected_type << std::endl;
        return OptionalChannelRef();
    }
    return channel;
}
#else //OLD_RETRIEVAL

const Bifrost::API::Channel *
get_channel(const Bifrost::API::Component& i_component,
				 const std::string& i_channel_name,
				 const Bifrost::API::DataType& i_expected_type)
{
    int channelIndex = findChannelIndexViaName(i_component,i_channel_name.c_str());
    if (channelIndex<0)
    {
        std::cerr << boost::format("get_channel() : channel '%1%' not found") % i_channel_name << std::endl;
        return nullptr;
    }
    const Bifrost::API::Channel* channel_ptr = &(i_component.channels()[channelIndex]);
    if (!channel_ptr->valid())
    {
        std::cerr << boost::format("get_channel() : channel '%1%' not valid") % i_channel_name << std::endl;
        return nullptr;
    }
    if ( channel_ptr->dataType() != i_expected_type)
    {
        std::cerr << boost::format("get_channel() : channel '%1%' of type %2% is different from expected type %3%") % i_channel_name % channel_ptr->dataType() % i_expected_type << std::endl;
        return nullptr;
    }
    return channel_ptr;
}
#endif //OLD_RETRIEVAL

bool process_liquid_point_component(const Bifrost::API::Component& component,
									const std::string& position_channel_name,
									const std::string& velocity_channel_name,
									const std::string& density_channel_name,
									const std::string& vorticity_channel_name,
									const std::string& droplet_channel_name,
									Imath::Box3f& bounds,
									uint32_t tsidx,
									Alembic::AbcGeom::OXform& xform)
{
    // Position channel
#ifdef OLD_CHANNEL_RETRIEVAL
    int positionChannelIndex = findChannelIndexViaName(component,position_channel_name.c_str());
    if (positionChannelIndex<0)
        return false;
    const Bifrost::API::Channel& position_ch = component.channels()[positionChannelIndex];
    if (!position_ch.valid())
        return false;
    if ( position_ch.dataType() != Bifrost::API::FloatV3Type)
        return false;
#else // OLD_CHANNEL_RETRIEVAL
    const Bifrost::API::Channel& position_ch_ptr = get_channel(component,position_channel_name,Bifrost::API::FloatV3Type);
    if (get_channel(component,position_channel_name,Bifrost::API::FloatV3Type))
    else
    	return false;
    if (!position_ch)
    	return false;
#endif // OLD_CHANNEL_RETRIEVAL

    if (false)
    {
        // Density channel
        int densityChannelIndex = findChannelIndexViaName(component,density_channel_name.c_str());
        if (densityChannelIndex<0)
            return false;
        const Bifrost::API::Channel& density_ch = component.channels()[densityChannelIndex];
        if (!density_ch.valid())
            return false;
        if ( density_ch.dataType() != Bifrost::API::FloatType)
            return false;

    }
    // Velocity channel
    int velocityChannelIndex = findChannelIndexViaName(component,velocity_channel_name.c_str());
    if (velocityChannelIndex<0)
        return false;
    const Bifrost::API::Channel& velocity_ch = component.channels()[velocityChannelIndex];
    if (!velocity_ch.valid())
        return false;
    if ( velocity_ch.dataType() != Bifrost::API::FloatV3Type)
        return false;

    // Create the OPoints object
    Alembic::AbcGeom::OPoints partsOut(xform,component.name().c_str(),tsidx);
    Alembic::AbcGeom::OPointsSchema &pSchema = partsOut.getSchema();

    Alembic::AbcGeom::MetaData mdata;
    SetGeometryScope( mdata, Alembic::AbcGeom::kVaryingScope );
    Alembic::AbcGeom::OV3fArrayProperty velOut( pSchema, ".velocities", mdata, tsidx );

    std::vector< Alembic::Abc::V3f > positions;
    std::vector< Alembic::Abc::V3f > velocities;
    std::vector< Alembic::Util::uint64_t > ids;

    // NOTE : Other than position, velocity and id, all the other information
    //        are expected to be store as arbGeomParam

    // Data accumulation
    Bifrost::API::Layout layout = component.layout();
    size_t depthCount = layout.depthCount();
    Alembic::Util::uint64_t currentId = 0;
    for ( size_t d=0; d<depthCount; d++ ) {
        for ( size_t t=0; t<layout.tileCount(d); t++ ) {
            Bifrost::API::TreeIndex tindex(t,d);
            if ( !position_ch.elementCount( tindex ) ) {
                // nothing there
                continue;
            }

            const Bifrost::API::TileData<amino::Math::vec3f>& position_tile_data = position_ch.tileData<amino::Math::vec3f>( tindex );
            const Bifrost::API::TileData<amino::Math::vec3f>& velocity_tile_data = velocity_ch.tileData<amino::Math::vec3f>( tindex );
            if (position_tile_data.count() != velocity_tile_data.count())
            {
                std::cerr << boost::format("Point position and velocity tile data count mismatch position[%1%] velocity[%2%]") % position_tile_data.count() % velocity_tile_data.count()<< std::endl;
                return false;
            }
            for (size_t i=0; i<position_tile_data.count(); i++ )
            {
                Imath::V3f point_position(position_tile_data[i][0],
										  position_tile_data[i][1],
										  position_tile_data[i][2]);
                positions.push_back(point_position);
                bounds.extendBy(point_position);
                ids.push_back(currentId);
                currentId++;
            }

            for (size_t i=0; i<velocity_tile_data.count(); i++ )
            {
                velocities.push_back(Imath::V3f (velocity_tile_data[i][0],
												 velocity_tile_data[i][1],
												 velocity_tile_data[i][2]));
            }
        }
    }

    // Update Alembic storage
    Alembic::AbcGeom::V3fArraySample position_data ( positions );
    Alembic::AbcGeom::UInt64ArraySample id_data ( ids );
    Alembic::AbcGeom::OPointsSchema::Sample psamp(position_data,
												  id_data);
    pSchema.set( psamp );
    velOut.set( Alembic::AbcGeom::V3fArraySample( velocities ) );
    return true;
}

bool process_bifrost_file(const std::string& bifrost_filename,
                          const std::string& alembic_filename,
                          const std::string& position_channel_name,
                          const std::string& velocity_channel_name,
						  const std::string& density_channel_name,
						  const std::string& vorticity_channel_name,
						  const std::string& droplet_channel_name)
{

    Bifrost::API::String biffile = bifrost_filename.c_str();
    Bifrost::API::ObjectModel om;
    Bifrost::API::FileIO fileio = om.createFileIO( biffile );
    const Bifrost::API::BIF::FileInfo& info = fileio.info();
    bool is_bifrost_foam_file(false);

    // Need to determine is the BIF file contains Foam or Liquid particle info
    if (std::string(info.componentName.c_str()).find("Foam")!=std::string::npos)
    	is_bifrost_foam_file = true;
    
    // Need to load the entire file's content to process
    Bifrost::API::StateServer ss = fileio.load( );
    if (ss.valid())
    {
        size_t numComponents = ss.components().count();
        if (numComponents>0)
        {
            /*
             * Create the Alembic file only if we get a valid state server
             * after loading and there is at least one component
             */

            Alembic::AbcGeom::OArchive archive(Alembic::Abc::CreateArchiveWithInfo(Alembic::AbcCoreHDF5::WriteArchive(),
                                                                                   alembic_filename.c_str(),
                                                                                   std::string("Procedural Insight Pty. Ltd."),
                                                                                   std::string("info@proceduralinsight.com")));
            Alembic::AbcGeom::OObject topObj( archive, Alembic::AbcGeom::kTop );
            Alembic::AbcGeom::OXform xform = addXform(topObj,"bif2abc");

            // Create the time sampling
            Alembic::Abc::chrono_t fps = 24.0;
            Alembic::Abc::chrono_t startTime = 0.0;
            Alembic::Abc::chrono_t iFps = 1.0/fps;
            Alembic::Abc::TimeSampling ts(iFps,startTime);
            uint32_t tsidx = topObj.getArchive().addTimeSampling(ts);

            for (size_t componentIndex=0;componentIndex<numComponents;componentIndex++)
            {
                Bifrost::API::Component component = ss.components()[componentIndex];
                Bifrost::API::TypeID componentType = component.type();
                if (componentType == Bifrost::API::PointComponentType)
                {
                    Imath::Box3f bounds;
                    process_liquid_point_component(component,
                    							   position_channel_name,
												   velocity_channel_name,
												   density_channel_name,
												   vorticity_channel_name,
												   droplet_channel_name,
												   bounds,tsidx,xform);
                }
            }
        }
    }
    else
    {
        std::cerr << boost::format("Unable to load the content of the Bifrost file \"%1%\"") % bifrost_filename.c_str()
                  << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char **argv)
{

    try {
        std::string density_channel_name("density");
        std::string position_channel_name("position");
        std::string velocity_channel_name("velocity");
        std::string vorticity_channel_name("vorticity");
        std::string droplet_channel_name("droplet");
        std::string bifrost_filename;
        std::string alembic_filename;
        float fps = 24.0f;
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "Produce help message")
            ("fps", po::value<float>(&fps),
             "Frames per second to scale velocity when determining the velocity-attenuated bounding box. Defaults to 24.0")
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
            ("abc", po::value<std::string>(&alembic_filename),
             "Alembic file. [Required]")
            ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        po::notify(vm);

        if (vm.count("help") || bifrost_filename.empty() || alembic_filename.empty()) {
            std::cout << desc << "\n";
            return 1;
        }

        process_bifrost_file(bifrost_filename,
                             alembic_filename,
                             position_channel_name,
                             velocity_channel_name,
							 density_channel_name,
							 vorticity_channel_name,
							 droplet_channel_name);
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
