#pragma once

#include <ai.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <string>

/*!
 * \brief Procedural arguments container
 * \note No need for to be a full fledge class, a struct is sufficient
 */
struct ProcArgs {
    typedef std::vector<struct AtNode *> AtNodePtrContainer;
    ProcArgs();
    AtNode * proceduralNode;
    AtNodePtrContainer createdNodes;
    float velocityScale;
    float pointRadius;
    size_t pointMode;
    bool enableVelocityMotionBlur;
    bool performEmission;
    std::string bifrostFilename;
    size_t bifrostTileIndex;
    size_t bifrostTileDepth;
    int processDataStringAsArgcArgv(int argc, const char **argv);
    void print() const;
};
