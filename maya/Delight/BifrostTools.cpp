#include "BifrostSurfaceShape.h"
#include "BifrostSurfaceShapeUI.h"
#include "BifrostSurfaceShapeCacheCommand.h"
#include <maya/MFnPlugin.h>
#include "MayaUtils.h"

MStatus initializePlugin( MObject obj )
{
  MStatus status;

  char versionStrBuf[BUFSIZ]="1.1.1";
  // sprintf(versionStrBuf,"Version %d.%d.%d, compiled %s on %s",MAJOR_VERSION_NUMBER,MINOR_VERSION_NUMBER,PATCH_VERSION_NUMBER,__TIME__,__DATE__);

  MFnPlugin plugin( obj,
                    "Procedural Insight Pty. Ltd. info@procedualinsight.com",
                    versionStrBuf,
                    "Any");
  CMS(status = plugin.registerShape( "BifrostSurfaceShape",
                                     BifrostSurfaceShape::typeId,
                                     &BifrostSurfaceShape::creator,
                                     &BifrostSurfaceShape::initialize,
                                     &BifrostSurfaceShapeUI::creator));

  CMS(status = plugin.registerCommand("BifrostSurfaceShapeCache",
                                      &BifrostSurfaceShapeCacheCommand::creator,
                                      &BifrostSurfaceShapeCacheCommand::newSyntax));

  MString load_path;
  CMS(load_path = plugin.loadPath(&status));
  // MGlobal::displayInfo(load_path);

  char commandStringBuffer[BUFSIZ];
  sprintf(commandStringBuffer,"pi_BifrostToolsMenu(\"%s\");",load_path.asChar());
  // MGlobal::displayInfo(MString("commandStringBuffer = ") + MString(commandStringBuffer));
  CMS(status = MGlobal::executeCommand( "source BifrostDelightIntegration.mel;"));
  // CMS(status = MGlobal::executeCommand( "pi_BifrostToolsMenu();"));
  CMS(status = MGlobal::executeCommand( commandStringBuffer));

  return status;
}

MStatus uninitializePlugin( MObject obj)
{
  MStatus status;
  int lic_status = 0;
  MFnPlugin plugin( obj );
  status = plugin.deregisterNode( BifrostSurfaceShape::typeId );
  
  return status;
}
