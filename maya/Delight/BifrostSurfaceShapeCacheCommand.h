#pragma once

#include <iostream>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MStringArray.h>
#include <maya/MObject.h>
#include <map>

// Requirements from 3Delight
const char SampleTime_Flag_ShortName[] = "-st";
const char SampleTime_Flag_LongName[] = "-sampleTime";
const char AddStep_Flag_ShortName[] = "-a";
const char AddStep_Flag_LongName[] = "-addstep";
const char Remove_Flag_ShortName[] = "-r";
const char Remove_Flag_LongName[] = "-remove";
const char Emit_Flag_ShortName[] = "-e";
const char Emit_Flag_LongName[] = "-emit";
const char Flush_Flag_ShortName[] = "-f";
const char Flush_Flag_LongName[] = "-flush";
const char Contains_Flag_ShortName[] = "-c";
const char Contains_Flag_LongName[] = "-contains";
const char List_Flag_ShortName[] = "-l";
const char List_Flag_LongName[] = "-list";

/*!
 * \brief Maya command for use by 3Delight to export procedural
 * \note See section 7.8 of the 3Delight for Maya manual (PDF) for details.
 *       The node must conform to the specification as dictated in the documentation to work
 */
class BifrostSurfaceShapeCacheCommand : public MPxCommand
{
public:
    BifrostSurfaceShapeCacheCommand();
    virtual ~BifrostSurfaceShapeCacheCommand();
    // Maya's requirement
    virtual MStatus doIt ( const MArgList& args );
    static void* creator();
    static MSyntax newSyntax();
private:
    MStatus parseSyntax (MArgDatabase &argData);
    bool isCached (MString &shapeName);
    void emitShader(MString &baseDirectory);
    static MStringArray m_CachedShapeNames;
    static MObject m_CurrentBifrostSurfaceShape;
};
