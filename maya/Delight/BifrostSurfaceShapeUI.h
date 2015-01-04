#pragma once

#include <maya/MPxSurfaceShapeUI.h>

class BifrostSurfaceShapeUI : public MPxSurfaceShapeUI {
public:
    BifrostSurfaceShapeUI();
    virtual ~BifrostSurfaceShapeUI();

   void  getDrawRequestsOld( const MDrawInfo & info,
                                   bool objectAndActiveOnly,
                                   MDrawRequestQueue & queue );

    virtual void  getDrawRequests( const MDrawInfo & info,
                                   bool objectAndActiveOnly,
                                   MDrawRequestQueue & queue );

	void			getDrawRequestsPoints( MDrawRequest&,
											  const MDrawInfo& );
	void			getDrawRequestsWireframe( MDrawRequest&,
											  const MDrawInfo& );
	void			getDrawRequestsBoundingBox( MDrawRequest&,
												const MDrawInfo& );
	void			getDrawRequestsShaded(	  MDrawRequest&,
											  const MDrawInfo&,
											  MDrawRequestQueue&,
											  MDrawData& data );

    virtual void  draw( const MDrawRequest & request,
                        M3dView & view ) const;

    virtual bool  select( MSelectInfo &selectInfo,
                          MSelectionList &selectionList,
                          MPointArray &worldSpaceSelectPts ) const;


    static  void *          creator();
    static  MTypeId         typeId;
private:
	// Draw Tokens
	//
	enum {
		kDrawWireframe,
		kDrawWireframeOnShaded,
		kDrawSmoothShaded,
		kDrawFlatShaded,
		kDrawBoundingBox,
		kDrawPoints,
		kLastToken
	};
};
