#pragma once

#include <ai.h>
#include <vector>

/*!
 * \brief Procedural arguments container
 * \note No need for to be a full fledge class, a struct is sufficient
 */
struct ProcArgs {
    ProcArgs();
    AtNode * proceduralNode;
    std::vector<struct AtNode *> createdNodes;
};
