#include "ProcArgs.h"
#include <ai.h>
#include <string.h>
#include <boost/format.hpp>
#include <math.h>

const size_t MAX_BIF_FILENAME_LENGTH = 4096;

int ProcInit( struct AtNode *node, void **user_ptr )
{
    ProcArgs * args = new ProcArgs();
    args->proceduralNode = node;

    std::string dataString = AiNodeGetStr(node,"data");
    const float current_frame = AiNodeGetFlt(AiUniverseGetOptions(), "frame");

    if (dataString.size() != 0)
    {
        std::string bif_filename_format = dataString;
        char bif_filename[MAX_BIF_FILENAME_LENGTH];
        uint32_t bif_int_frame_number = static_cast<uint32_t>(floor(current_frame));
        int sprintf_status = sprintf(bif_filename,bif_filename_format.c_str(),bif_int_frame_number);

        std::cerr << boost::format("BIFROST ARNOLD PROCEDURAL : bif filename = %1%, frame = %2%")
            % bif_filename % current_frame << std::endl;

        // Do stuff here
        args->createdNodes.push_back(AiNode("sphere"));
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
