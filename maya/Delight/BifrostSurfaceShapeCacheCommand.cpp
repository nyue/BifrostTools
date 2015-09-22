#include "BifrostSurfaceShapeCacheCommand.h"
#include <maya/MGlobal.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MTime.h>
#include "MayaUtils.h"
#include "BifrostSurfaceShape.h"
#include <ri.h>
#include <fstream>
#include <boost/format.hpp>

MStringArray BifrostSurfaceShapeCacheCommand::m_CachedShapeNames;
MObject BifrostSurfaceShapeCacheCommand::m_CurrentBifrostSurfaceShape;


BifrostSurfaceShapeCacheCommand::BifrostSurfaceShapeCacheCommand()
{
}

BifrostSurfaceShapeCacheCommand::~BifrostSurfaceShapeCacheCommand()
{
}

MStatus BifrostSurfaceShapeCacheCommand::doIt ( const MArgList& args )
{
    MStatus status = MS::kSuccess;

    MArgDatabase argsDB ( syntax(), args );
    CMS(status = parseSyntax(argsDB));


    return status;
}

void* BifrostSurfaceShapeCacheCommand::creator()
{
    return new BifrostSurfaceShapeCacheCommand();
}

/*!
 * [from section 7.8 of 3Delight for Maya PDF manual]
 * cache_command -addStep -sampleTime <double> <shape>
 *   The command is expected to keep a sample of the specified object at the current
 *   time, which is passed via the -sampleTime flag; the command can assume that
 *   Maya���s current time is already set to this value when it is called. The command
 *   should store a combination of the object���s name, topology, sample time. No
 *   return value is expected.
 * cache_command -list
 *   Return the names of the shapes that have been cached by this command in a
 *   string array.
 * cache_command -flush
 *   Clear all cached data. No return value is expected.
 * cache_command -contains <shape>
 *   Returns true if the specified object has been cached, regardless of the sample
 *   time. Returns false otherwise.
 * cache_command -remove <shape>
 *   Removes the specified object from the cache. No return value is expected.
 * cache_command -emit <shape>
 *   Issues the Ri calls that will go inside the ObjectBegin/End or
 *   ArchiveBegin/End block for the specified object. If the object can be deformation blurred,
 *   it should produce the proper motion blocks. No return value is expected.
 *
 */
MSyntax BifrostSurfaceShapeCacheCommand::newSyntax()
{
	MSyntax syntax;
	MStatus status = MS::kSuccess;

	syntax.addFlag(SampleTime_Flag_ShortName, SampleTime_Flag_LongName, MSyntax::kDouble, MSyntax::kSelectionItem);
	syntax.addFlag(AddStep_Flag_ShortName,    AddStep_Flag_LongName);
	syntax.addFlag(Remove_Flag_ShortName,     Remove_Flag_LongName,     MSyntax::kSelectionItem);
	syntax.addFlag(Emit_Flag_ShortName,       Emit_Flag_LongName,       MSyntax::kSelectionItem);
	syntax.addFlag(Flush_Flag_ShortName,      Flush_Flag_LongName                       );
	syntax.addFlag(Contains_Flag_ShortName,   Contains_Flag_LongName,   MSyntax::kSelectionItem);
	syntax.addFlag(List_Flag_ShortName,       List_Flag_LongName                        );
	syntax.setObjectType(MSyntax::kSelectionList, 0, 1);

	return syntax;
}

MStatus BifrostSurfaceShapeCacheCommand::parseSyntax (MArgDatabase &argData)
{
	MStatus status = MS::kSuccess;
	double sampleTime = 0;
	MString shapeName = "";
	MSelectionList shapes;

	/*
	RtContextHandle currentContext = RiGetContext();
	printf("currentContext = %p\n",currentContext);
	RiContext(0);
	RiBegin(RI_NULL);
	*/
	if (argData.isFlagSet(SampleTime_Flag_ShortName)) {
		argData.getFlagArgument( SampleTime_Flag_ShortName, 0, sampleTime);
		char sampleTimeMessage[BUFSIZ];
		sprintf(sampleTimeMessage,"BifrostSurfaceShapeCacheCommand : SampleTime = %f",sampleTime);
		MGlobal::displayInfo(sampleTimeMessage);
		std::cout << sampleTimeMessage << std::endl;
		MStringArray shapeNameArray;
		CMS(status = argData.getFlagArgument( SampleTime_Flag_ShortName, 1, shapes));
		MDagPath dagPath;
		CMS(status = shapes.getSelectionStrings(shapeNameArray));
		CMS(status = shapes.getDagPath(0,dagPath));
// #ifndef NDEBUG
		MGlobal::displayInfo("BifrostSurfaceShape to process : START ");
		for (unsigned int i=0;i<shapeNameArray.length();i++) {
			MGlobal::displayInfo(shapeNameArray[i]);
		}
		MGlobal::displayInfo("BifrostSurfaceShape to process : END ");
// #endif // NDEBUG
		shapeName = shapeNameArray[0];
		CMS(shapeName = dagPath.fullPathName(&status));
	}

	if (argData.isFlagSet(AddStep_Flag_ShortName)) {
		// CMS(status = argData.getFlagArgument( AddStep_Flag_ShortName, 0, shapes));
		MGlobal::displayInfo(MString("BifrostSurfaceShapeCacheCommand : AddStep appending shape name = ") + shapeName);
		std::cout << "BifrostSurfaceShapeCacheCommand : AddStep " << std::endl;
		{
			/* Names must be unique, find out if the name is already cached
			   This is CRITICAL if not the motion block data will have multiple
			   copies
			*/
			unsigned int numCachedShape = m_CachedShapeNames.length();
			bool wasCached = false;
			for (unsigned int i=0;i<numCachedShape;i++) {
				if ( shapeName == m_CachedShapeNames[i]) {
					wasCached = true;
					break;
				}
			}
			if (!wasCached) {
				MGlobal::displayInfo(MString("BifrostSurfaceShapeCacheCommand : CACHING = ") + shapeName);
				CMS(status = m_CachedShapeNames.append(shapeName));
			}
		}
		// MObject depNode;
		CMS(status = shapes.getDependNode(0,m_CurrentBifrostSurfaceShape));
		{
			unsigned int numShapes = m_CachedShapeNames.length();
			std::cout << "addstep numShapes = " << numShapes << std::endl;
		}

		// Stash away motion block data for use when Emit is called
		CMS(MFnDependencyNode nodeFn(m_CurrentBifrostSurfaceShape,&status));
		if (nodeFn.typeName() != "BifrostSurfaceShape") {
			MGlobal::displayError("Non BifrostSurfaceShape chosen in BifrostSurfaceShapeCacheCommand AddStep");
			return MS::kFailure;
		}

		CMS(BifrostSurfaceShape* pBifrostSS = static_cast<BifrostSurfaceShape*>(nodeFn.userNode(&status)));
		if (pBifrostSS) {
			pBifrostSS->update();
			pBifrostSS->accumulateMotionData();
		}

	}

	if (argData.isFlagSet(Remove_Flag_ShortName)) {
		CMS(status = argData.getFlagArgument( Remove_Flag_ShortName, 0, shapeName));
		MGlobal::displayInfo(MString("BifrostSurfaceShapeCacheCommand : Remove shape name = ") + shapeName);
		std::cout << "BifrostSurfaceShapeCacheCommand : Remove" << std::endl;
		unsigned int length = m_CachedShapeNames.length();
		for (unsigned int i=0;i<length;i++) {
			if (m_CachedShapeNames[i] == shapeName) {
				CMS(status = m_CachedShapeNames.remove(i));
				break;
			}
		}
	}

	if (argData.isFlagSet(Emit_Flag_ShortName)) {
		CMS(status = argData.getFlagArgument( Emit_Flag_ShortName, 0, shapeName));
		MGlobal::displayInfo(MString("BifrostSurfaceShapeCacheCommand : Emit called for shapename = ") + shapeName);

		MSelectionList emittingSelectionList;
		CMS(status = emittingSelectionList.add(shapeName));
		MObject emttingBifrostSurfaceShape;
		CMS(status = emittingSelectionList.getDependNode(0,emttingBifrostSurfaceShape));
		CMS(MFnDependencyNode nodeFn(emttingBifrostSurfaceShape,&status));
		if (nodeFn.typeName() != "BifrostSurfaceShape") {
			MGlobal::displayError("Non BifrostSurfaceShape chosen for emtting RIB via 3Delight");
			return MS::kFailure;
		}
      
		CMS(BifrostSurfaceShape* pBifrostSS = static_cast<BifrostSurfaceShape*>(nodeFn.userNode(&status)));
		char dsoArgs[16384] = {""};
    
		// Bifrost cache file path
		MString empFileAttrName("BifrostFile");
		CMS(MPlug empFilePlug = nodeFn.findPlug(empFileAttrName,true,&status));
		MString empFilePath;
		CMS(status = empFilePlug.getValue(empFilePath));
    
		// Current time so as to obtain the frame number for picking up the correct Bifrost cache
		/*
		  Maxime @3delight.com suggested using "delightRenderState -qf" for the integer frame number
		 */
		MString empTimeAttrName("time");
		CMS(MPlug empTimePlug = nodeFn.findPlug(empTimeAttrName,true,&status));
		MTime empTimeValue;
		CMS(status = empTimePlug.getValue(empTimeValue));
		int frameNumber = static_cast<int>(ceil(empTimeValue.as( empTimeValue.uiUnit() )));
		{
			char buf[BUFSIZ];
			sprintf(buf,"Bifrost CacheCommand Procedural frame number = %d, animation time = %f",
					frameNumber,
					empTimeValue.as( empTimeValue.uiUnit() ));
			MGlobal::displayInfo(buf);
		}
		char numberedFrameBifrostFilePath[2048];
		sprintf(numberedFrameBifrostFilePath,empFilePath.asChar(),frameNumber);
		MGlobal::displayInfo(MString("BifrostSurfaceShapeCacheCommand : Emit RiAttributeBegin"));

		/*
		// Width for RIB if generating point geometry
		MString empWidthAttrName("width");
		CMS(MPlug empWidthPlug = nodeFn.findPlug(empWidthAttrName,true,&status));
		double empWidthValue;
		CMS(status = empWidthPlug.getValue(empWidthValue));
		{
			char buf[BUFSIZ];
			sprintf(buf,"Bifrost CacheCommand Procedural width = %f",empWidthValue);
			MGlobal::displayInfo(buf);
		}
		*/

		// Check if we should use velocity for motion blur
		MString empVelocityBlurAttrName("velocityBlur");
		CMS(MPlug empVelocityBlurPlug = nodeFn.findPlug(empVelocityBlurAttrName,true,&status));
		bool empVelocityBlurValue;
		CMS(status = empVelocityBlurPlug.getValue(empVelocityBlurValue));
		{
			char buf[BUFSIZ];
			sprintf(buf,"Bifrost CacheCommand Procedural velocity blur = %s",empVelocityBlurValue?"true":"false");
			MGlobal::displayInfo(buf);
		}

		// Geometry chunk size for emitting geometry in the DSO
		MString empChunkAttrName("chunk");
		CMS(MPlug empChunkPlug = nodeFn.findPlug(empChunkAttrName,true,&status));
		int empChunkValue;
		CMS(status = empChunkPlug.getValue(empChunkValue));
		{
			char buf[BUFSIZ];
			sprintf(buf,"Bifrost CacheCommand Procedural chunk = %d",empChunkValue);
			MGlobal::displayInfo(buf);
		}

		// Build the DSO arguments with the input parameter values
		sprintf(dsoArgs,"--emp_file %s "
				// "--width %f "
				"--velocity_blur %d --chunk_size %d",
				numberedFrameBifrostFilePath,
				// empWidthValue,
				empVelocityBlurValue?1:0,
				empChunkValue);

		CMS(status = MGlobal::executeCommand( "RiAttributeBegin;"));
		bool hasGeometry = pBifrostSS->hasGeometryData();
		if (hasGeometry)
		{
			CMS(status = MGlobal::executeCommand( "RiArchiveRecord -mode \"comment\" -text \"CustomSurfaceShapeCacheCommand emit command begins\";"));
			CMS(status = MGlobal::executeCommand( "RiReverseOrientation;"));
			bool RIUseRunProgram = true;
			RtString data[2] = {"CustomProcedural",dsoArgs};
			RtBound bound = {-RI_INFINITY,RI_INFINITY,
							 -RI_INFINITY,RI_INFINITY,
							 -RI_INFINITY,RI_INFINITY};
			RtString procedural_data[2] = {"emp_runproc",dsoArgs};
			RtString program_data[2] = {"emp_runprog",dsoArgs};
			char proceduralCommandString[BUFSIZ];
			if (RIUseRunProgram)
			{
				sprintf(proceduralCommandString,
						"RiProcedural -programName emp_runprog -bound %f %f %f %f %f %f -param \"%s\";",
						bound[0],
						bound[1],
						bound[2],
						bound[3],
						bound[4],
						bound[5],
						dsoArgs
						);
				MGlobal::displayInfo(proceduralCommandString);
				MGlobal::executeCommand( proceduralCommandString );
			}
			else
			{
				sprintf(proceduralCommandString,
						"RiProcedural -libraryName emp_runproc -bound %f %f %f %f %f %f -param \"%s\";",
						bound[0],
						bound[1],
						bound[2],
						bound[3],
						bound[4],
						bound[5],
						dsoArgs
						);
				MGlobal::displayInfo(proceduralCommandString);
				MGlobal::executeCommand( proceduralCommandString );
			}
		}
		bool hasField = pBifrostSS->hasFieldData();
		if (hasField)
		{
			char RiInteriorCommandString[BUFSIZ];

			// RiInterior -n "empField" -p "empFile" "string" "abc.emp" -p "fieldName" "string" "temperature";
			sprintf(RiInteriorCommandString,"RiInterior -n \"empField\" -p \"empFile\" \"string\" \"%s\" -p \"fieldName\" \"string\" \"%s\";",
					numberedFrameBifrostFilePath,"temperature");
			CMS(status = MGlobal::executeCommand(RiInteriorCommandString));

			CMS(status = MGlobal::executeCommand( "RiOpacity -o 0 0 0;"));
			char RiBBoxCommandString[BUFSIZ];

			MBoundingBox bbox = pBifrostSS->boundingBox();

			sprintf(RiBBoxCommandString,"RiBBox(%f,%f,%f,%f,%f,%f);",
					bbox.min().x,bbox.min().y,bbox.min().z,
					bbox.max().x,bbox.max().y,bbox.max().z);
			MGlobal::displayInfo(RiBBoxCommandString);
			CMS(status = MGlobal::executeCommand( RiBBoxCommandString ));

			bool doBlobby = false;
			if (doBlobby)
			{
				char proceduralCommandString[BUFSIZ];
				char RiBlobbyCommandString[BUFSIZ];
				/*
				  Blobby 1 [1004 0 0 0 1 1] [0] ["emp_impfld.so" "/home/nicholas/projects/ExoticMatter/graphs/LKSmoke/emp/field/smoke.0100.emp"] "varying float temperature" []
				  
				*/
				RtInt nleaf = 1;
				RtInt nentry = 0;
				RtInt entry[] = {0};
				RtInt nfloat = 0;
				RtFloat floats[] = {0.0f};
				RtInt nstring = 2;
				RtString strings[] = {"emp_impfld.so",numberedFrameBifrostFilePath};
				// RiSphere(1,-1,1,360,RI_NULL);
				// RiArchiveRecord(RI_COMMENT,"NICHOLAS",RI_NULL);
				CMS(status = MGlobal::executeCommand( "RiInterior -n \"noisysmoke\";"));
				CMS(status = MGlobal::executeCommand( "RiAttribute -n \"shade\" -p \"volumeintersectionstrategy\" \"string\" \"additive\";"));
				CMS(status = MGlobal::executeCommand( "RiOpacity -o 0 0 0;"));
				
				sprintf(RiBlobbyCommandString,"    Blobby 1 [1004 0 0 0 1 1] [0] [\\\"emp_impfld.so\\\" \\\"%s\\\"] \\\"varying float temperature\\\" [0]\\n",numberedFrameBifrostFilePath);
				sprintf(proceduralCommandString,"RiArchiveRecord -m \"verbatim\" -t \"%s\";",RiBlobbyCommandString);
				// sprintf(proceduralCommandString,"RiArchiveRecord -m \"verbatim\" -t \"HELLO WORLD\n\"");
				MGlobal::displayInfo(proceduralCommandString);
				CMS(status = MGlobal::executeCommand( proceduralCommandString ));
				/*
				  RiBlobby( nleaf,
				  nentry,  entry,
				  nfloat,  floats,
				  nstring, strings,
				  RI_NULL);
				*/
			}
		}
		CMS(status = MGlobal::executeCommand( "RiAttributeEnd;"));

		MGlobal::displayInfo(MString("BifrostSurfaceShapeCacheCommand : Emit RiAttributeEnd"));
		
		if (pBifrostSS) {
			pBifrostSS->clearMotionData();
		}
	}

	if (argData.isFlagSet(Flush_Flag_ShortName)) {

		MGlobal::displayInfo("Flush cached shapenames and shapenodes");
		std::cout << "BifrostSurfaceShapeCacheCommand : Flush cached shapenames and shapenodes" << std::endl;

		CMS(status = m_CachedShapeNames.clear());

	}

	if (argData.isFlagSet(Contains_Flag_ShortName)) {
		CMS(status = argData.getFlagArgument( Contains_Flag_ShortName, 0, shapeName));
		MGlobal::displayInfo(MString("Contains checking for shape name : ") + shapeName);
		setResult(isCached(shapeName));
	}

	if (argData.isFlagSet(List_Flag_ShortName)) {
#ifndef NDEBUG
		MGlobal::displayInfo("Listing all shape names");
		unsigned int numShapes = m_CachedShapeNames.length();
		std::cout << "numShapes = " << numShapes << std::endl;
		for (unsigned int i=0;i<numShapes;i++) {
			MGlobal::displayInfo(MString("\t") + m_CachedShapeNames[i]);
		}
#endif // NDEBUG
		setResult(m_CachedShapeNames);
	}
	// RiEnd();
	return status;
}

bool BifrostSurfaceShapeCacheCommand::isCached (MString &shapeName)
{
	unsigned int length = m_CachedShapeNames.length();
	for (unsigned int i=0;i<length;i++) {
		if (m_CachedShapeNames[i] == shapeName) {
			MGlobal::displayInfo(MString("Cache lookup succeeded for ") + shapeName );
			return true;
		}
	}
	MGlobal::displayInfo(MString("Cache lookup failed for ") + shapeName );
	return false;
}

void BifrostSurfaceShapeCacheCommand::emitShader (MString &baseDirectory)
{
}
