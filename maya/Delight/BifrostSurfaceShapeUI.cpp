#include "BifrostSurfaceShape.h"
#include "BifrostSurfaceShapeUI.h"
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>
#include <maya/MDrawData.h>
#include <maya/MMaterial.h>

// Maya color
const int LEAD_COLOR            = 18; // green
const int ACTIVE_COLOR          = 15; // white
const int ACTIVE_AFFECTED_COLOR =  8; // purple
const int DORMANT_COLOR         =  4; // blue
const int HILITE_COLOR          = 17; // pale blue

MTypeId BifrostSurfaceShapeUItypeId(0x0011BDC1);

BifrostSurfaceShapeUI::BifrostSurfaceShapeUI()
{
}

BifrostSurfaceShapeUI::~BifrostSurfaceShapeUI()
{
}

void  BifrostSurfaceShapeUI::getDrawRequests( const MDrawInfo & info,
                                          bool objectAndActiveOnly,
                                          MDrawRequestQueue & queue )
{
	// The draw data is used to pass geometry through the 
	// draw queue. The data should hold all the information
	// needed to draw the shape.
	//
	MDrawData data;
	MDrawRequest request = info.getPrototype( *this );
    /*
	quadricShape* shapeNode = (quadricShape*)surfaceShape();
	quadricGeom* geom = shapeNode->geometry();
	getDrawData( geom, data );
	request.setDrawData( data );
    */
	// Are we displaying meshes?
	if ( ! info.objectDisplayStatus( M3dView::kDisplayMeshes ) )
		return;

	// Use display status to determine what color to draw the object
	//
	switch ( info.displayStyle() )
    {		 
		case M3dView::kWireFrame :
			getDrawRequestsWireframe( request, info );
			queue.add( request );
			break;
		
		case M3dView::kBoundingBox :
			getDrawRequestsBoundingBox( request, info );
			queue.add( request );
			break;
		
		case M3dView::kGouraudShaded :
			request.setToken( kDrawSmoothShaded );
			getDrawRequestsShaded( request, info, queue, data );
			queue.add( request );
			break;
		
		case M3dView::kFlatShaded :
			request.setToken( kDrawFlatShaded );
 			getDrawRequestsShaded( request, info, queue, data );
			queue.add( request );
			break;

		case M3dView::kPoints :
			getDrawRequestsPoints( request, info );
			queue.add( request );
			break;


		default:	
			break;
    }
}

void BifrostSurfaceShapeUI::getDrawRequestsPoints( MDrawRequest& request,
                                                  const MDrawInfo& info )
{
	request.setToken( kDrawPoints );

	M3dView::DisplayStatus displayStatus = info.displayStatus();
	M3dView::ColorTable activeColorTable = M3dView::kActiveColors;
	M3dView::ColorTable dormantColorTable = M3dView::kDormantColors;
	switch ( displayStatus )
    {
		case M3dView::kLead :
			request.setColor( LEAD_COLOR, activeColorTable );
			break;
		case M3dView::kActive :
			request.setColor( ACTIVE_COLOR, activeColorTable );
			break;
		case M3dView::kActiveAffected :
			request.setColor( ACTIVE_AFFECTED_COLOR, activeColorTable );
			break;
		case M3dView::kDormant :
			request.setColor( DORMANT_COLOR, dormantColorTable );
			break;
		case M3dView::kHilite :
			request.setColor( HILITE_COLOR, activeColorTable );
			break;
		default:
			break;
    }
}



void BifrostSurfaceShapeUI::getDrawRequestsWireframe( MDrawRequest& request,
                                                  const MDrawInfo& info )
{
	request.setToken( kDrawWireframe );

	M3dView::DisplayStatus displayStatus = info.displayStatus();
	M3dView::ColorTable activeColorTable = M3dView::kActiveColors;
	M3dView::ColorTable dormantColorTable = M3dView::kDormantColors;
	switch ( displayStatus )
    {
		case M3dView::kLead :
			request.setColor( LEAD_COLOR, activeColorTable );
			break;
		case M3dView::kActive :
			request.setColor( ACTIVE_COLOR, activeColorTable );
			break;
		case M3dView::kActiveAffected :
			request.setColor( ACTIVE_AFFECTED_COLOR, activeColorTable );
			break;
		case M3dView::kDormant :
			request.setColor( DORMANT_COLOR, dormantColorTable );
			break;
		case M3dView::kHilite :
			request.setColor( HILITE_COLOR, activeColorTable );
			break;
		default:	
			break;
    }
}

void BifrostSurfaceShapeUI::getDrawRequestsBoundingBox( MDrawRequest& request,
													const MDrawInfo& info )
{
	request.setToken( kDrawBoundingBox );

	M3dView::DisplayStatus displayStatus = info.displayStatus();
	M3dView::ColorTable activeColorTable = M3dView::kActiveColors;
	M3dView::ColorTable dormantColorTable = M3dView::kDormantColors;
	switch ( displayStatus )
    {
		case M3dView::kLead :
			request.setColor( LEAD_COLOR, activeColorTable );
			break;
		case M3dView::kActive :
			request.setColor( ACTIVE_COLOR, activeColorTable );
			break;
		case M3dView::kActiveAffected :
			request.setColor( ACTIVE_AFFECTED_COLOR, activeColorTable );
			break;
		case M3dView::kDormant :
			request.setColor( DORMANT_COLOR, dormantColorTable );
			break;
		case M3dView::kHilite :
			request.setColor( HILITE_COLOR, activeColorTable );
			break;
		default:	
			break;
    }
}

void BifrostSurfaceShapeUI::getDrawRequestsShaded( MDrawRequest& request,
                                               const MDrawInfo& info,
                                               MDrawRequestQueue& queue,
                                               MDrawData& data )
{
	M3dView::DisplayStatus displayStatus = info.displayStatus();
#ifdef ENABLE_MAYA_MATERIAL
	// Need to get the material info
	//
	MDagPath path = info.multiPath();	// path to your dag object 
	M3dView view = info.view();; 		// view to draw to
	MMaterial material = MPxSurfaceShapeUI::material( path );

	// Evaluate the material and if necessary, the texture.
	//
	if ( ! material.evaluateMaterial( view, path ) ) {
		cerr << "Couldnt evaluate\n";
	}

	bool drawTexture = true;
	if ( drawTexture && material.materialIsTextured() ) {
		material.evaluateTexture( data );
	}

	request.setMaterial( material );

	bool materialTransparent = false;
	material.getHasTransparency( materialTransparent );
	if ( materialTransparent ) {
		request.setIsTransparent( true );
	}
	
#endif // ENABLE_MAYA_MATERIAL

	// create a draw request for wireframe on shaded if
	// necessary.
	//
	if ( (displayStatus == M3dView::kActive) ||
       (displayStatus == M3dView::kLead) ||
       (displayStatus == M3dView::kHilite) )
    {
      MDrawRequest wireRequest = info.getPrototype( *this );
      wireRequest.setDrawData( data );
      getDrawRequestsWireframe( wireRequest, info );
      wireRequest.setToken( kDrawWireframeOnShaded );
      wireRequest.setDisplayStyle( M3dView::kWireFrame );
      queue.add( wireRequest );
    }
}

void  BifrostSurfaceShapeUI::getDrawRequestsOld( const MDrawInfo & info,
										  bool objectAndActiveOnly,
										  MDrawRequestQueue & queue )
{
	// MGlobal::displayInfo("BifrostSurfaceShapeUI::getDrawRequests");
	MDrawRequest request = info.getPrototype(*this);
	// Set up color
	M3dView::DisplayStatus displayStatus = info.displayStatus();
	switch ( displayStatus )
    {
    case M3dView::kLead :
		request.setColor( LEAD_COLOR, M3dView::kActiveColors );
		break;
    case M3dView::kActive :
		request.setColor( ACTIVE_COLOR, M3dView::kActiveColors );
		break;
    case M3dView::kActiveAffected :
		request.setColor( ACTIVE_AFFECTED_COLOR, M3dView::kActiveColors );
		break;
    case M3dView::kDormant :
		request.setColor( DORMANT_COLOR, M3dView::kDormantColors );
		break;
    case M3dView::kHilite :
		request.setColor( HILITE_COLOR, M3dView::kActiveColors );
		break;
    default:
		break;
    }
	queue.add(request);
}

void  BifrostSurfaceShapeUI::draw( const MDrawRequest & request,
							   M3dView & view ) const
{
	// MGlobal::displayInfo("BifrostSurfaceShapeUI::draw");

	MDagPath path;
	M3dView::DisplayStyle style;
	M3dView::DisplayStatus dstatus;
	BifrostSurfaceShape *shapeNode = (BifrostSurfaceShape *)surfaceShape();
	if (shapeNode)
	{
		shapeNode->update();
		shapeNode->draw(view,path,request.displayStyle(),request.displayStatus());
	}
}

bool  BifrostSurfaceShapeUI::select( MSelectInfo &selectInfo,
								 MSelectionList &selectionList,
								 MPointArray &worldSpaceSelectPts ) const
{
	MGlobal::displayInfo("BifrostSurfaceShapeUI::select");

    MSelectionMask priorityMask( MSelectionMask::kSelectObjectsMask );
    MSelectionList item;
    item.add( selectInfo.selectPath() );
    MPoint xformedPt;
    selectInfo.addSelection(item, xformedPt, selectionList,
                            worldSpaceSelectPts, priorityMask, false );
	return true;
}

void* BifrostSurfaceShapeUI::creator()
{
	return new BifrostSurfaceShapeUI();
}

