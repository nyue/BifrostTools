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

Alembic::AbcGeom::OXform
addXform(Alembic::Abc::OObject parent, std::string name)
{
    Alembic::AbcGeom::OXform xform(parent, name.c_str());

    return xform;
}

bool process_point_component(const Bifrost::API::Component& component,
                            const std::string& position_channel_name,
                            const std::string& velocity_channel_name,
                            Imath::Box3f& bounds,
                            uint32_t tsidx,
                            Alembic::AbcGeom::OXform& xform)
{
    // Position channel
    int positionChannelIndex = findChannelIndexViaName(component,position_channel_name.c_str());
    if (positionChannelIndex<0)
        return false;
    const Bifrost::API::Channel& position_ch = component.channels()[positionChannelIndex];
    if (!position_ch.valid())
        return false;
    if ( position_ch.dataType() != Bifrost::API::FloatV3Type)
        return false;

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
                          const std::string& velocity_channel_name)
{

    Bifrost::API::String biffile = bifrost_filename.c_str();
    Bifrost::API::ObjectModel om;
    Bifrost::API::FileIO fileio = om.createFileIO( biffile );
    const Bifrost::API::BIF::FileInfo& info = fileio.info();

//     std::cout << boost::format("Version        : %1%") % info.version << std::endl;
//     std::cout << boost::format("Frame          : %1%") % info.frame << std::endl;
//     std::cout << boost::format("Channel count  : %1%") % info.channelCount << std::endl;
//     std::cout << boost::format("Component name : %1%") % info.componentName.c_str() << std::endl;
//     std::cout << boost::format("Component type : %1%") % info.componentType << std::endl;
//     std::cout << boost::format("Object name    : %1%") % info.objectName.c_str() << std::endl;
//     std::cout << boost::format("Layout name    : %1%") % info.layoutName.c_str() << std::endl;

//     for (size_t channelIndex=0;channelIndex<info.channelCount;channelIndex++)
//     {
//         std::cout << std::endl;
//         const Bifrost::API::BIF::FileInfo::ChannelInfo& channelInfo = fileio.channelInfo(channelIndex);
//         std::cout << boost::format("        Channel name  : %1%") % channelInfo.name.c_str() << std::endl;
//         std::cout << boost::format("        Data type     : %1%") % channelInfo.dataType << std::endl;
//         std::cout << boost::format("        Max depth     : %1%") % channelInfo.maxDepth << std::endl;
//         std::cout << boost::format("        Tile count    : %1%") % channelInfo.tileCount << std::endl;
//         std::cout << boost::format("        Element count : %1%") % channelInfo.elementCount << std::endl;
//     }

    
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

            Alembic::AbcGeom::OArchive archive(
                                               Alembic::Abc::CreateArchiveWithInfo(Alembic::AbcCoreHDF5::WriteArchive(),
                                                                                   alembic_filename.c_str(),
                                                                                   std::string("Procedural Insight Pty. Ltd."),
                                                                                   std::string("info@proceduralinsight.com"))
                                               );
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
                    process_point_component(component, position_channel_name,velocity_channel_name,bounds,tsidx,xform);
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
        std::string position_channel_name("position");
        std::string velocity_channel_name("velocity");
        std::string bifrost_filename;
        std::string alembic_filename;
        float fps = 24.0f;
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "Produce help message")
            ("fps", po::value<float>(&fps),
             "Frames per second to scale velocity when determining the velocity-attenuated bounding box. Defaults to 24.0")
            ("input-file,i", po::value<std::string>(&bifrost_filename),
             "Bifrost file")
            ("output-file,o", po::value<std::string>(&alembic_filename),
             "Alembic file")
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
                             velocity_channel_name);
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
