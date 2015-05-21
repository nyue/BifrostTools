#include "ProcArgs.h"
#include <utils/BifrostUtils.h>
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
// #include <bifrostapi/bifrost_stringarray.h>
// #include <bifrostapi/bifrost_refarray.h>
#include <bifrostapi/bifrost_channel.h>
#include <bifrostapi/bifrost_layout.h>
// Bifrost headers - END

const size_t MAX_BIF_FILENAME_LENGTH = 4096;

int ProcInit( struct AtNode *node, void **user_ptr )
{
    // printf("ProcInit : 0001\n");
    ProcArgs * args = new ProcArgs();
    args->proceduralNode = node;

    const char *parentProceduralDSO = AiNodeGetStr(node,"dso");
    std::string dataString = AiNodeGetStr(node,"data");
    // printf("ProcInit : 0002 dataString = \"%s\"\n",dataString.c_str());
    if (dataString.size() != 0)
    {
        // printf("ProcInit : 0010\n");
        const float current_frame = AiNodeGetFlt(AiUniverseGetOptions(), "frame");
        const float fps_1 = 1.0f/AiNodeGetFlt(AiUniverseGetOptions(), "fps");

        std::string parsingDataString = (boost::format("%1% %2%") % parentProceduralDSO % dataString.c_str()).str();
        PI::String2ArgcArgv s2aa(parsingDataString);
        args->processDataStringAsArgcArgv(s2aa.argc(),s2aa.argv());
        // printf("ProcInit : 0015\n");
        // args->print();
        // printf("ProcInit : 0016\n");
        std::string bif_filename_format = args->bifrostFilename;

        char bif_filename[MAX_BIF_FILENAME_LENGTH];
        uint32_t bif_int_frame_number = static_cast<uint32_t>(floor(current_frame));
        int sprintf_status = sprintf(bif_filename,bif_filename_format.c_str(),bif_int_frame_number);
        // printf("ProcInit : 0018 bif_filename_format = \"%s\"\n",bif_filename_format.c_str());

        // printf("ProcInit : 0030\n");
        /*!
         * \remark Iterate through each tile in the bifrost file,
         *         determine the bounds for that tile (including
         *         velocity blur growth) and generate a procedural
         *         for that tile of particle data
         */
        // printf("ProcInit : 0031 bif_filename = \"%s\"\n",bif_filename);
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
            // printf("ProcInit : 0040\n");
            Bifrost::API::Component component = ss.components()[componentIndex];
            Bifrost::API::TypeID componentType = component.type();
            if (componentType == Bifrost::API::PointComponentType)
            {
                // printf("ProcInit : 0050\n");
                int positionChannelIndex = findChannelIndexViaName(component,"position");
                int velocityChannelIndex = findChannelIndexViaName(component,"velocity");
                if (positionChannelIndex>=0)
                {
                    // printf("ProcInit : 0060\n");
                    const Bifrost::API::Channel& position_ch = component.channels()[positionChannelIndex];
                    const Bifrost::API::Channel& velocity_ch = component.channels()[velocityChannelIndex];
                    if (position_ch.valid()
                        &&
                        (args->enableVelocityMotionBlur?velocity_ch.valid():true) // check conditionally
                        )
                    {
                        // printf("ProcInit : 0070\n");
                        // iterate over the tile tree at each level
                        Bifrost::API::Layout layout = component.layout();
                        size_t depthCount = layout.depthCount();
                        for ( size_t d=0; d<depthCount; d++ ) {
                            for ( size_t t=0; t<layout.tileCount(d); t++ ) {
                                Bifrost::API::TreeIndex tindex(t,d);
                                if ( !position_ch.elementCount( tindex ) ) {
                                    // nothing there
                                    continue;
                                }

                                if ( position_ch.dataType() == Bifrost::API::FloatV3Type
                                     &&
                                     (args->enableVelocityMotionBlur?(velocity_ch.dataType() == Bifrost::API::FloatV3Type):true) // check conditionally
                                     )
                                {
                                    args->createdNodes.push_back(AiNode("points"));
                                    AtNode *points = args->createdNodes.back();
                                    const Bifrost::API::TileData<amino::Math::vec3f>& position_tile_data = position_ch.tileData<amino::Math::vec3f>( tindex );
                                    const Bifrost::API::TileData<amino::Math::vec3f>& velocity_tile_data = velocity_ch.tileData<amino::Math::vec3f>( tindex );
                                    if (position_tile_data.count() == velocity_tile_data.count())
                                    {
                                        std::vector<amino::Math::vec3f> P(position_tile_data.count());
                                        std::vector<amino::Math::vec3f> PP;
                                        if (args->enableVelocityMotionBlur)
                                            PP.resize(position_tile_data.count());
                                        std::vector<float> radius(position_tile_data.count(),args->pointRadius);
                                        for (size_t i=0; i<position_tile_data.count(); i++ ) {
                                            P[i] = position_tile_data[i];
                                            if (args->enableVelocityMotionBlur)
                                            {
                                                PP[i][0] = P[i][0] +  args->velocityScale * fps_1 * velocity_tile_data[i][0];
                                                PP[i][1] = P[i][1] +  args->velocityScale * fps_1 * velocity_tile_data[i][1];
                                                PP[i][2] = P[i][2] +  args->velocityScale * fps_1 * velocity_tile_data[i][2];
                                            }
                                        }
                                        if (args->enableVelocityMotionBlur)
                                        {
                                            AtArray *vlistArray = 0;
                                            vlistArray = AiArrayAllocate(P.size(),2,AI_TYPE_POINT);
                                            
                                            AiArraySetKey(vlistArray, 0, &(P[0]));
                                            AiArraySetKey(vlistArray, 1, &(PP[0]));
                                            AiNodeSetArray(points, "points",vlistArray);
                                        }
                                        else
                                        {
                                            AiNodeSetArray(points, "points",
                                                           AiArrayConvert(P.size(),1,AI_TYPE_POINT,&(P[0])));
                                        }
                                        AiNodeSetArray(points, "radius",
                                                       AiArrayConvert(radius.size(),1,AI_TYPE_FLOAT,&(radius[0])));
                                        AiNodeSetInt(points,"mode",args->pointMode);
                                        
                                    }
                                }
                                else
                                {
                                    AiMsgWarning("Bifrost-procedural : Position channel not of FloatV3Type or velocity channel not of FloatV3Type where velocity motion blur is requested");
                                }
                            }
                        }
                    }
                    else
                    {
                        AiMsgWarning("Bifrost-procedural : Position channel not found or velocity channel not found where velocity motion blur is requested");
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
    int numNodes = (int) args->createdNodes.size();
    // printf("ProcNumNodes : numNodes = %d\n",numNodes);
    return numNodes;
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
