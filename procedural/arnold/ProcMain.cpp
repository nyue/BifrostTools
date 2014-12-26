#include "ProcArgs.h"
#include <ai.h>
#include <string.h>
#include <boost/format.hpp>
#include <math.h>
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
                                 ProcArgs * args)
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
                                args->createdNodes.push_back(AiNode("points"));
                                AtNode *points = args->createdNodes.back();
                                const Bifrost::API::TileData<amino::Math::vec3f>& f3 = ch.tileData<amino::Math::vec3f>( tindex );
                                std::vector<amino::Math::vec3f> P(f3.count());
                                std::vector<float> radius(f3.count(),0.1f);
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
    ProcArgs * args = new ProcArgs();
    args->proceduralNode = node;

    std::string dataString = AiNodeGetStr(node,"data");
    const float current_frame = AiNodeGetFlt(AiUniverseGetOptions(), "frame");
    const float fps = AiNodeGetFlt(AiUniverseGetOptions(), "fps");

    if (dataString.size() != 0)
    {
        std::string bif_filename_format = dataString;
        char bif_filename[MAX_BIF_FILENAME_LENGTH];
        uint32_t bif_int_frame_number = static_cast<uint32_t>(floor(current_frame));
        int sprintf_status = sprintf(bif_filename,bif_filename_format.c_str(),bif_int_frame_number);

        std::cerr << boost::format("BIFROST ARNOLD PROCEDURAL : bif filename = %1%, frame = %2%, fps = %3%")
            % bif_filename % current_frame % fps << std::endl;

        // Do stuff here
        ProcessBifrostParticleCache(bif_filename,args);

        // args->createdNodes.push_back(AiNode("sphere"));
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
