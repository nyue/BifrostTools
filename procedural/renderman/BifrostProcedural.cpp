#include <ri.h>
#include <stdlib.h>
#include <iostream>
#include <boost/format.hpp>
#include <utils/BifrostUtils.h>

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
 * \brief Put everything into a single class for easier memory management
 */
struct BifrostProceduralParameters
{
    BifrostProceduralParameters()
    : fps(24.0f)
    , enableVelocityMotionBlur(true)
    , velocityScale(1.0f)
    , pointRadius(1.f)
    {}
    virtual ~BifrostProceduralParameters() {}
    std::string bifrost_filename;
    float fps;
    bool enableVelocityMotionBlur;
    float velocityScale;
    float pointRadius;

};

void EmitGeometry(float radius)
{
  RiSphere(radius,-radius,radius,360.0f,RI_NULL);
}

bool process_bifrost(const BifrostProceduralParameters& bifrost_params, RtFloat detail)
{
    float fps_1 = 1.0f/bifrost_params.fps;
    Bifrost::API::String biffile = bifrost_params.bifrost_filename.c_str();
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
                    (bifrost_params.enableVelocityMotionBlur?velocity_ch.valid():true) // check conditionally
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
                                 (bifrost_params.enableVelocityMotionBlur?(velocity_ch.dataType() == Bifrost::API::FloatV3Type):true) // check conditionally
                                 )
                            {
//                                args->createdNodes.push_back(AiNode("points"));
//                                AtNode *points = args->createdNodes.back();
                                const Bifrost::API::TileData<amino::Math::vec3f>& position_tile_data = position_ch.tileData<amino::Math::vec3f>( tindex );
                                const Bifrost::API::TileData<amino::Math::vec3f>& velocity_tile_data = velocity_ch.tileData<amino::Math::vec3f>( tindex );
                                if (position_tile_data.count() == velocity_tile_data.count())
                                {
                                    std::vector<amino::Math::vec3f> P(position_tile_data.count());
                                    std::vector<amino::Math::vec3f> PP;
                                    if (bifrost_params.enableVelocityMotionBlur)
                                        PP.resize(position_tile_data.count());
                                    std::vector<float> radius(position_tile_data.count(),bifrost_params.pointRadius);
                                    for (size_t i=0; i<position_tile_data.count(); i++ ) {
                                        P[i] = position_tile_data[i];
                                        if (bifrost_params.enableVelocityMotionBlur)
                                        {
                                            PP[i][0] = P[i][0] +  bifrost_params.velocityScale * fps_1 * velocity_tile_data[i][0];
                                            PP[i][1] = P[i][1] +  bifrost_params.velocityScale * fps_1 * velocity_tile_data[i][1];
                                            PP[i][2] = P[i][2] +  bifrost_params.velocityScale * fps_1 * velocity_tile_data[i][2];
                                        }
                                    }
                                    if (bifrost_params.enableVelocityMotionBlur)
                                    {
                                        // args->pointMode
                                        RtString point_type("disk");
                                        RtFloat mbTime[2] = {-0.2f,0.2f};
                                        RiMotionBeginV(2,mbTime);
                                        RtFloat width = 2.0f * bifrost_params.pointRadius;
                                        RiPoints(P.size(),RI_P,&(P[0]),RI_CONSTANTWIDTH,&width,
                                                "uniform string type",&point_type,
                                                RI_NULL);
                                        RiPoints(PP.size(),RI_P,&(PP[0]),RI_CONSTANTWIDTH,&width,
                                                "uniform string type",&point_type,
                                                RI_NULL);
                                        RiMotionEnd();
                                    }
                                    else
                                    {
                                        // args->pointMode
                                        RtFloat width = 2.0f * bifrost_params.pointRadius;
                                        RtString point_type("blobby");
                                        RiPoints(P.size(),RI_P,&(P[0]),RI_CONSTANTWIDTH,&(width),
                                                // "uniform string type",&point_type,
                                                RI_NULL);
                                    }
                                }
                            }
                            else
                            {
                                ;
//                                AiMsgWarning("Bifrost-procedural : Position channel not of FloatV3Type or velocity channel not of FloatV3Type where velocity motion blur is requested");
                            }
                        }
                    }
                }
                else
                {
                    ;
//                    AiMsgWarning("Bifrost-procedural : Position channel not found or velocity channel not found where velocity motion blur is requested");
                }
            }

        }
    }
    return true;
}

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*

  Do these as part of linking stage via CMake
  configuration

#ifdef _WIN32
#define EXTERN extern _declspec(dllexport)
#else
#define EXTERN extern
#endif

EXTERN RtPointer ConvertParameters(RtString paramstr);
EXTERN RtVoid Subdivide(RtPointer data, RtFloat detail);
EXTERN RtVoid Free(RtPointer data);
*/

RtPointer ConvertParameters(RtString paramstr)
{
    std::cerr << "ConvertParameters" << std::endl;
    std::string ri_param(paramstr);
    std::cerr << boost::format("ri_param = \"%1%\"") % ri_param.c_str() << std::endl;

    BifrostProceduralParameters *param = new BifrostProceduralParameters();
    param->bifrost_filename = ri_param;
    return (RtPointer)param;
}

RtVoid Subdivide(RtPointer data, RtFloat detail)
{
    const BifrostProceduralParameters *param = (BifrostProceduralParameters *)data;
    std::cerr << boost::format("param->bifrost_filename \"%1%\"") % param->bifrost_filename.c_str() << std::endl;

    process_bifrost(*param, detail);

}

RtVoid Free(RtPointer data)
{
    BifrostProceduralParameters *param = (BifrostProceduralParameters *)data;
    delete param;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
