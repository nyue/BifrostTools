#include <bifrostapi/bifrost_om.h>
#include <bifrostapi/bifrost_stateserver.h>
#include <bifrostapi/bifrost_component.h>
#include <bifrostapi/bifrost_fileio.h>
#include <bifrostapi/bifrost_fileutils.h>
#include <bifrostapi/bifrost_string.h>
// #include <bifrostapi/bifrost_stringarray.h>
#include <bifrostapi/bifrost_refarray.h>
#include <bifrostapi/bifrost_channel.h>
#include <bifrostapi/bifrost_layout.h>
#include <boost/format.hpp>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr,"Usage : %s <bifrost file>\n",argv[0]);
        exit(1);
    }
    Bifrost::API::String biffile = argv[1];
    Bifrost::API::ObjectModel om;
    Bifrost::API::FileIO fileio = om.createFileIO( biffile );
    Bifrost::API::StateServer ss = fileio.load( );

    if ( !ss.valid() ) {
        std::cerr << "bifinfo : file loading error" << std::endl;
        exit(1);
    }

    size_t numComponents = ss.components().count();
    std::cout << "Number of components : " << numComponents << std::endl;
    for (size_t i=0;i<numComponents;i++)
    {
        Bifrost::API::Component component = ss.components()[i];
        Bifrost::API::TypeID componentType = component.type();
        if (componentType == Bifrost::API::PointComponentType)
        {
            std::cout << "Point component : "
                    << component.name().c_str()
                    << std::endl;

            Bifrost::API::RefArray channels = component.channels();
            size_t particleCount = component.elementCount();
            size_t channelCount = channels.count();
            for (size_t channelIndex=0;channelIndex<channelCount;channelIndex++)
            {
                const Bifrost::API::Channel& ch = channels[channelIndex];
                Bifrost::API::String channelName = ch.name();
                Bifrost::API::DataType channelDataType = ch.dataType();
                std::cout << boost::format("\tChannel[%1%] of type %2% : %3% has %4% particles")
                % channelIndex % channelDataType % channelName.c_str() % channelCount << std::endl;
                bool doDump = true;
                if (doDump)
                {
                    // iterate over the tile tree at each level
                    Bifrost::API::Layout layout = component.layout();
                    size_t depthCount = layout.depthCount();
                    for ( size_t d=0; d<depthCount; d++ ) {
                        for ( size_t t=0; t<layout.tileCount(d); t++ ) {
                            Bifrost::API::TreeIndex tindex(t,d);
                            if ( !ch.elementCount( tindex ) ) {
                                // nothing there
                                continue;
                            }
                            std::cout << "tile:" << t << " depth:" << d << std::endl;
                            switch (channelDataType)
                            {
                            case Bifrost::API::FloatType :
                            {
                                const Bifrost::API::TileData<float>& f1 = ch.tileData<float>( tindex );
                                for (size_t i=0; i<f1.count(); i++ ) {
                                    const float& val = f1[i];
                                    std::cout << "\t" << f1[i] << std::endl;
                                }

                                std::cout << std::endl;
                            }
                            break;
                            case Bifrost::API::FloatV2Type :
                                break;
                            case Bifrost::API::FloatV3Type :
                            {
                                const Bifrost::API::TileData<amino::Math::vec3f>& f3 = ch.tileData<amino::Math::vec3f>( tindex );
                                for (size_t i=0; i<f3.count(); i++ ) {
                                    const amino::Math::vec3f& val = f3[i];
                                    std::cout << "\t" << val[0] << " " << val[1] << " " << val[2] << std::endl;
                                }

                                std::cout << std::endl;
                            }
                            break;
                            case Bifrost::API::Int32Type :
                                break;
                            case Bifrost::API::Int64Type :
                                break;
                            case Bifrost::API::UInt32Type :
                                break;
                            case Bifrost::API::UInt64Type :
                                break;
                            case Bifrost::API::Int32V2Type :
                                break;
                            case Bifrost::API::Int32V3Type :
                                break;
                            default:
                                std::cerr << "Unknown channel type encountered" << std::endl;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else if (componentType == Bifrost::API::VoxelComponentType)
        {
            std::cout << "Voxel component : "
                    << component.name().c_str()
                    << std::endl;
            Bifrost::API::RefArray channels = component.channels();
            size_t channelCount = channels.count();
            for (size_t channelIndex=0;channelIndex<channelCount;channelIndex++)
            {
                Bifrost::API::String channelName = Bifrost::API::Base(channels[channelIndex]).name();
                std::cout << "\tChannel : "
                        << channelName.c_str()
                        << std::endl;
            }
        }
    }

    exit(0);
}
