#include "ProcArgs.h"
#include <ai.h>
#include <string.h>
#include <boost/format.hpp>
#include <math.h>
#include <String2ArgcArgv.h>
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
#include <bifrostapi/bifrost_layout.h>
// Bifrost headers - END

/*!
 * DESIGN : The procedural recurse one level. Root level does splitting,
 *          subsequent level does geometry emission
 */
const size_t MAX_BIF_FILENAME_LENGTH = 4096;

int findChannelIndexViaName(const Bifrost::API::Component& component,
                            const Bifrost::API::String& searchChannelName)
{
    Bifrost::API::RefArray channels = component.channels();
    size_t particleCount = component.elementCount();
    size_t channelCount = channels.count();
    for (size_t channelIndex=0;channelIndex<channelCount;channelIndex++)
    {
        const Bifrost::API::Channel& ch = channels[channelIndex];
        Bifrost::API::String channelName = ch.name();
        if (channelName.find(searchChannelName) != Bifrost::API::String::npos)
        {
            //            std::cout << boost::format("ARNOLD BIFROST FOUND CHANNEL %1% MATCHING %2%")
            //            % channelName.c_str()
            //            % searchChannelName.c_str()
            //            << std::endl;
            return channelIndex;
        }
    }
    return -1;
}

bool ProcessBifrostParticleCache(const std::string& bif_filename,
        const float pointRadius,
        ProcArgs::AtNodePtrContainer & createdNodes)
{
    Bifrost::API::String biffile = bif_filename.c_str();
    Bifrost::API::ObjectModel om;
    Bifrost::API::FileIO fileio = om.createFileIO( biffile );
    Bifrost::API::StateServer ss = fileio.load( );
    if ( !ss.valid() ) {
        return false;
    }

    size_t numComponents = ss.components().count();
    for (size_t componentIndex=0;componentIndex<numComponents;componentIndex++)
    {
        Bifrost::API::Component component = ss.components()[componentIndex];
        Bifrost::API::TypeID componentType = component.type();
        if (componentType == Bifrost::API::PointComponentType)
        {
            int channelIndex = findChannelIndexViaName(component,"position");
            if (channelIndex>=0)
            {
                const Bifrost::API::Channel& ch = component.channels()[channelIndex];
                if (ch.valid())
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

                            // std::cout << "tile:" << t << " depth:" << d << std::endl;
                            if ( ch.dataType() == Bifrost::API::FloatV3Type )
                            {
                                createdNodes.push_back(AiNode("points"));
                                AtNode *points = createdNodes.back();
                                const Bifrost::API::TileData<amino::Math::vec3f>& f3 = ch.tileData<amino::Math::vec3f>( tindex );
                                std::vector<amino::Math::vec3f> P(f3.count());
                                std::vector<float> radius(f3.count(),pointRadius);
                                for (size_t i=0; i<f3.count(); i++ ) {
                                    const amino::Math::vec3f& val = f3[i];
                                    // std::cerr << "\t" << val[0] << " " << val[1] << " " << val[2] << std::endl;
                                    P[i] = f3[i];
                                }
                                AiNodeSetArray(points, "points",
                                                           AiArrayConvert(P.size(),1,AI_TYPE_POINT,&(P[0])));
                                AiNodeSetArray(points, "radius",
                                                           AiArrayConvert(radius.size(),1,AI_TYPE_FLOAT,&(radius[0])));
                            }

                        }
                    }

                }
            }
 
        }
    }
    
    return true;
}

int ProcInit( struct AtNode *node, void **user_ptr )
{
    printf("ProcInit : 0001\n");
    ProcArgs * args = new ProcArgs();
    args->proceduralNode = node;

    const char *parentProceduralDSO = AiNodeGetStr(node,"dso");
    std::string dataString = AiNodeGetStr(node,"data");
    printf("ProcInit : 0002 dataString = \"%s\"\n",dataString.c_str());
    if (dataString.size() != 0)
    {
        printf("ProcInit : 0010\n");
        const float current_frame = AiNodeGetFlt(AiUniverseGetOptions(), "frame");
        const float fps = AiNodeGetFlt(AiUniverseGetOptions(), "fps");

        std::string parsingDataString = (boost::format("%1% %2%") % parentProceduralDSO % dataString.c_str()).str();
        PI::String2ArgcArgv s2aa(parsingDataString);
        args->processDataStringAsArgcArgv(s2aa.argc(),s2aa.argv());
        printf("ProcInit : 0015\n");

        std::string bif_filename_format = args->bifrostFilename;

        char bif_filename[MAX_BIF_FILENAME_LENGTH];
        uint32_t bif_int_frame_number = static_cast<uint32_t>(floor(current_frame));
        int sprintf_status = sprintf(bif_filename,bif_filename_format.c_str(),bif_int_frame_number);
        printf("ProcInit : 0016 bif_filename_format = \"%s\"\n",bif_filename_format.c_str());

        if (args->performEmission)
        {
            printf("ProcInit : 0020\n");
            // Emit Arnold geometry
            std::cerr << boost::format("BIFROST ARNOLD PROCEDURAL : bif filename = %1%, frame = %2%, fps = %3%")
                % bif_filename % current_frame % fps << std::endl;

            ProcessBifrostParticleCache(bif_filename,args->pointRadius,args->createdNodes);
        }
        else
        {
            printf("ProcInit : 0030\n");
            /*!
             * \remark Iterate through each tile in the bifrost file,
             *         determine the bounds for that tile (including
             *         velocity blur growth) and generate a procedural
             *         for that tile of particle data
             */
            printf("ProcInit : 0031 bif_filename = \"%s\"\n",bif_filename);
            Bifrost::API::String biffile = bif_filename;
            Bifrost::API::ObjectModel om;
            Bifrost::API::FileIO fileio = om.createFileIO( biffile );
            Bifrost::API::StateServer ss = fileio.load( );
            if ( !ss.valid() ) {
                return false;
            }
            size_t proceduralIndex = 0;
            size_t numComponents = ss.components().count();
            for (size_t componentIndex=0;componentIndex<numComponents;componentIndex++)
            {
                printf("ProcInit : 0040\n");
                Bifrost::API::Component component = ss.components()[componentIndex];
                Bifrost::API::TypeID componentType = component.type();
                if (componentType == Bifrost::API::PointComponentType)
                {
                    printf("ProcInit : 0050\n");
                    int channelIndex = findChannelIndexViaName(component,"position");
                    if (channelIndex>=0)
                    {
                        printf("ProcInit : 0060\n");
                        const Bifrost::API::Channel& ch = component.channels()[channelIndex];
                        if (ch.valid())
                        {
                            printf("ProcInit : 0070\n");
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

                                    if ( ch.dataType() == Bifrost::API::FloatV3Type )
                                    {
                                        printf("ProcInit : 0080\n");
                                        Imath::Box3f particleBound;
                                        const Bifrost::API::TileData<amino::Math::vec3f>& f3 = ch.tileData<amino::Math::vec3f>( tindex );
                                        std::vector<amino::Math::vec3f> P(f3.count());
                                        for (size_t i=0; i<f3.count(); i++ ) {
                                            // const amino::Math::vec3f& val = f3[i];
                                            particleBound.extendBy(Imath::V3f(f3[i][0],f3[i][1],f3[i][2]));
                                        }


                                        args->createdNodes.push_back(AiNode("procedural"));
                                        AtNode *procedural = args->createdNodes.back();
                                        char proceduralName[256];
                                        sprintf(proceduralName,"Nested%04lu",proceduralIndex);
                                        AiNodeSetStr(procedural,"name",proceduralName);
                                        AiNodeSetStr(procedural,"dso",parentProceduralDSO);
                                        AiNodeSetBool(procedural,"load_at_init",false);
                                        /*
                                          ("radius", po::value<float>(&radius),
                                           "radius for RIB point geometry.")
                                          ("velocity-blur", "use velocity for motion blur.")
                                          ("bif", po::value<std::string>(&bifrost_filename),
                                           "bifrost filename.")
                                          ("tile-index", po::value<size_t>(&tileIndex),
                                           "bifrost tile index.")
                                          ("tile-depth", po::value<size_t>(&tileDepth),
                                           "bifrost tile depth.")
                                          ("emit", "non-root level, perform emission.")
                                         */
                                        boost::format formattedDataString = boost::format(
                                                "%1%" // implicitly contains --bif, --radius, --velocity-blur
                                                " --tile-index %2%"
                                                " --tile-depth %3%"
                                                " --emit")
                                                % dataString.c_str()
                                                % t
                                                % d;
                                        std::cerr << boost::format("Procedural data string : \"%1%\"") % formattedDataString.str().c_str();
                                        AiNodeSetStr(procedural,"data",formattedDataString.str().c_str());
#ifdef USESTUFF
                                        createdNodes.push_back(AiNode("points"));
                                        AtNode *points = createdNodes.back();
                                        const Bifrost::API::TileData<amino::Math::vec3f>& f3 = ch.tileData<amino::Math::vec3f>( tindex );
                                        std::vector<amino::Math::vec3f> P(f3.count());
                                        std::vector<float> radius(f3.count(),pointRadius);
                                        for (size_t i=0; i<f3.count(); i++ ) {
                                            const amino::Math::vec3f& val = f3[i];
                                            // std::cerr << "\t" << val[0] << " " << val[1] << " " << val[2] << std::endl;
                                            P[i] = f3[i];
                                        }
                                        AiNodeSetArray(points, "points",
                                                                   AiArrayConvert(P.size(),1,AI_TYPE_POINT,&(P[0])));
                                        AiNodeSetArray(points, "radius",
                                                                   AiArrayConvert(radius.size(),1,AI_TYPE_FLOAT,&(radius[0])));
#endif

                                    }

                                }
                            }

                        }
                    }

                }
            }

        }
    }

    *user_ptr = args;

    return true;
}

int ProcCleanup( void *user_ptr )
{
    delete reinterpret_cast<ProcArgs*>( user_ptr );

    return true;
}

int ProcNumNodes( void *user_ptr )
{
    ProcArgs * args = reinterpret_cast<ProcArgs*>( user_ptr );
    return (int) args->createdNodes.size();
}

struct AtNode* ProcGetNode(void *user_ptr, int i)
{
    ProcArgs * args = reinterpret_cast<ProcArgs*>( user_ptr );
	
    if ( i >= 0 && i < (int) args->createdNodes.size() )
    {
        return args->createdNodes[i];
    }
	
    return NULL;
}

extern "C"
{
    int ProcLoader(AtProcVtable* api)
    {
        api->Init        = ProcInit;
        api->Cleanup     = ProcCleanup;
        api->NumNodes    = ProcNumNodes;
        api->GetNode     = ProcGetNode;
        strcpy(api->version, AI_VERSION);
        return 1;
    }
}
