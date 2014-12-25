#ifdef BOILER_PLATE_CODE

int ProcInit( struct AtNode *node, void **user_ptr );
int ProcCleanup( void *user_ptr );
int ProcNumNodes( void *user_ptr );
struct AtNode* ProcGetNode(void *user_ptr, int i);

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

#endif // BOILER_PLATE_CODE

#include <ai.h>
#include <string.h>
#include "ProcArgs.h"

using namespace VII;

int ProcInit( struct AtNode *node, void **user_ptr )
{
    ProcArgs * args = new ProcArgs();
    args->proceduralNode = node;

	{
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
