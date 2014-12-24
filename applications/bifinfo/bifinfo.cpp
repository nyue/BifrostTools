#include <bifrostapi/bifrost_om.h>
#include <bifrostapi/bifrost_stateserver.h>
#include <bifrostapi/bifrost_component.h>
#include <bifrostapi/bifrost_fileio.h>
#include <bifrostapi/bifrost_fileutils.h>
#include <bifrostapi/bifrost_string.h>
#include <bifrostapi/bifrost_stringarray.h>
#include <bifrostapi/bifrost_refarray.h>

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
            size_t channelCount = channels.count();
            for (size_t channelIndex=0;channelIndex<channelCount;channelIndex++)
            {
            	Bifrost::API::String channelName = Bifrost::API::Base(channels[channelIndex]).name();
                std::cout << "\tChannel : "
                		<< channelName.c_str()
        				<< std::endl;
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
