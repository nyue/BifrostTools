#include <bifrostapi/bifrost_om.h>
#include <bifrostapi/bifrost_stateserver.h>
#include <bifrostapi/bifrost_component.h>
#include <bifrostapi/bifrost_fileio.h>
#include <bifrostapi/bifrost_fileutils.h>
#include <bifrostapi/bifrost_string.h>
#include <bifrostapi/bifrost_stringarray.h>
#include <bifrostapi/bifrost_refarray.h>
#include <bifrostapi/bifrost_channel.h>
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
    const Bifrost::API::BIF::FileInfo& info = fileio.info();

    std::cout << boost::format("Version        : %1%") % info.version << std::endl;
    std::cout << boost::format("Frame          : %1%") % info.frame << std::endl;
    std::cout << boost::format("Channel count  : %1%") % info.channelCount << std::endl;
    std::cout << boost::format("Component name : %1%") % info.componentName.c_str() << std::endl;
    std::cout << boost::format("Component type : %1%") % info.componentType << std::endl;
    std::cout << boost::format("Object name    : %1%") % info.objectName.c_str() << std::endl;
    std::cout << boost::format("Layout name    : %1%") % info.layoutName.c_str() << std::endl;

    for (size_t channelIndex=0;channelIndex<info.channelCount;channelIndex++)
    {
        std::cout << std::endl;
        const Bifrost::API::BIF::FileInfo::ChannelInfo& channelInfo = fileio.channelInfo(channelIndex);
        std::cout << boost::format("        Channel name  : %1%") % channelInfo.name.c_str() << std::endl;
        std::cout << boost::format("        Data type     : %1%") % channelInfo.dataType << std::endl;
        std::cout << boost::format("        Max depth     : %1%") % channelInfo.maxDepth << std::endl;
        std::cout << boost::format("        Tile count    : %1%") % channelInfo.tileCount << std::endl;
        std::cout << boost::format("        Element count : %1%") % channelInfo.elementCount << std::endl;
    }
    return 0;
}
