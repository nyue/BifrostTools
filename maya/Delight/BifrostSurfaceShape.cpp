#include "BifrostSurfaceShape.h"
#include <maya/MGlobal.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MPointArray.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MBoundingBox.h>
#include <maya/MTime.h>
#include <stdio.h>

#include "MayaUtils.h"

MTypeId BifrostSurfaceShape::typeId(0x0011BDC0);
MObject BifrostSurfaceShape::_inBifrostFileAttr;
MObject BifrostSurfaceShape::_inTimeAttr;
// MObject BifrostSurfaceShape::_inParticleWidthAttr;
MObject BifrostSurfaceShape::_inVelocityMotionBlurAttr;
MObject BifrostSurfaceShape::_inGeometryChunkAttr;
MObject BifrostSurfaceShape::_outChannelNamesAttr;
MObject BifrostSurfaceShape::_outBoundingBoxAttr;

BifrostSurfaceShape::BifrostSurfaceShape()
    : _particleBBox(MBoundingBox(MPoint(-1,-1,-1),MPoint(1,1,1)))
    , _hasParticleColor(false)
    , _hasParticleData(false)
{
}

BifrostSurfaceShape::~BifrostSurfaceShape()
{
}

// MPxSurfaceShape methods to over-ride (from Maya)
void BifrostSurfaceShape::postConstructor()
{
    setRenderable( true );
}

void BifrostSurfaceShape::draw (M3dView& view,
                                const MDagPath& path,
                                M3dView::DisplayStyle style,
                                M3dView::DisplayStatus status)
{
    switch (style)
    {
    case M3dView::kBoundingBox:
        {
            view.beginGL();
            glPushAttrib(GL_CURRENT_BIT);

            BodyMeshDataCollection::const_iterator mEiter = _bm.end();
            BodyMeshDataCollection::const_iterator mIter  = _bm.begin();
            for (;mIter!=mEiter;++mIter)
            {
                drawBBox(mIter->_meshBBox);
            }

            BodyFieldDataCollection::const_iterator fEiter = _bf.end();
            BodyFieldDataCollection::const_iterator fIter  = _bf.begin();
            for (;fIter!=fEiter;++fIter)
            {
                drawBBox(fIter->_fieldBBox);
            }

            if (_hasParticleData)
                drawBBox(_particleBBox);

            glPopAttrib();
            view.endGL();
            // MGlobal::displayInfo("Drawing bounding box");
        }
        break;
    case M3dView::kPoints:
        {
            // NOTE : Client state enable/disable order must reverse each other (like a stack)
            if (_hasParticleData)
            {
                view.beginGL();
                glPushAttrib(GL_CURRENT_BIT);
                glEnableClientState(GL_VERTEX_ARRAY);
                if (_hasParticleColor)
                    glEnableClientState(GL_COLOR_ARRAY);
                glVertexPointer(3,GL_FLOAT,0,&_particlePositions[0]);
                if (_hasParticleColor)
                    glColorPointer(3,GL_FLOAT,0,&_particleColors[0]);
                glDrawElements(GL_POINTS,_particlePositions.size()/3,GL_UNSIGNED_INT,&_particleGLIndices[0]);
                if (_hasParticleColor)
                    glDisableClientState(GL_COLOR_ARRAY);
                glDisableClientState(GL_VERTEX_ARRAY);
                glPopAttrib();
                view.endGL();
            }
            else if (_bm.size() != 0)
            {
                // Draw the mesh vertices as points
                view.beginGL();
                BodyMeshDataCollection::const_iterator mEiter = _bm.end();
                BodyMeshDataCollection::const_iterator mIter  = _bm.begin();
                for (;mIter!=mEiter;++mIter)
                {
                    glPushAttrib(GL_CURRENT_BIT);
                    glEnableClientState(GL_VERTEX_ARRAY);
                    if (mIter->_hasMeshVertexColor)
                        glEnableClientState(GL_COLOR_ARRAY);
                    glVertexPointer(3,GL_FLOAT,0,&(mIter->_meshPositions[0]));
                    if (mIter->_hasMeshVertexColor)
                        glColorPointer(3,GL_FLOAT,0,&(mIter->_meshColors[0]));
                    glDrawElements(GL_POINTS,mIter->_meshPositions.size()/3,GL_UNSIGNED_INT,
                                   &(mIter->_meshVertexGLIndices[0]));
                    if (mIter->_hasMeshVertexColor)
                        glDisableClientState(GL_COLOR_ARRAY);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glPopAttrib();
                }

                view.endGL();
            }
        }
        break;
    case M3dView::kWireFrame:
        {
            if (_bm.size() != 0)
            {
                view.beginGL();

                BodyMeshDataCollection::const_iterator mEiter = _bm.end();
                BodyMeshDataCollection::const_iterator mIter  = _bm.begin();
                for (;mIter!=mEiter;++mIter)
                {
                    glPushAttrib(GL_CURRENT_BIT);
                    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                    glEnableClientState(GL_VERTEX_ARRAY);
                    if (mIter->_hasMeshVertexColor)
                        glEnableClientState(GL_COLOR_ARRAY);
                    glVertexPointer(3,GL_FLOAT,0,&(mIter->_meshPositions[0]));
                    if (mIter->_hasMeshVertexColor)
                        glColorPointer(3,GL_FLOAT,0,&(mIter->_meshColors[0]));
                    glDrawElements(GL_TRIANGLES,mIter->_meshGLIndices.size(),GL_UNSIGNED_INT,
                                   &(mIter->_meshGLIndices[0]));
                    if (mIter->_hasMeshVertexColor)
                        glDisableClientState(GL_COLOR_ARRAY);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glPopAttrib();
                }
                view.endGL();
            }
        }
        break;
    case M3dView::kFlatShaded:
        {
            if (_bm.size() != 0)
            {
                view.beginGL();
                BodyMeshDataCollection::const_iterator mEiter = _bm.end();
                BodyMeshDataCollection::const_iterator mIter  = _bm.begin();
                for (;mIter!=mEiter;++mIter)
                {
                    glPushAttrib(GL_CURRENT_BIT);
                    glPolygonMode( GL_FRONT, GL_FILL );
                    glEnableClientState(GL_VERTEX_ARRAY);
                    if (mIter->_hasMeshVertexColor)
                        glEnableClientState(GL_COLOR_ARRAY);
                    glVertexPointer(3,GL_FLOAT,0,&(mIter->_meshPositions[0]));
                    if (mIter->_hasMeshVertexColor)
                        glColorPointer(3,GL_FLOAT,0,&(mIter->_meshColors[0]));
                    glDrawElements(GL_TRIANGLES,mIter->_meshGLIndices.size(),GL_UNSIGNED_INT,
                                   &(mIter->_meshGLIndices[0]));
                    if (mIter->_hasMeshVertexColor)
                        glDisableClientState(GL_COLOR_ARRAY);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glPopAttrib();
                }
                view.endGL();
            }
        }
        break;
    case M3dView::kGouraudShaded:
        {
            /*! \todo Need to handle vertex normal if available */
            if (_bm.size() != 0)
            {
                view.beginGL();
                BodyMeshDataCollection::const_iterator mEiter = _bm.end();
                BodyMeshDataCollection::const_iterator mIter  = _bm.begin();
                for (;mIter!=mEiter;++mIter)
                {
                    glPushAttrib(GL_CURRENT_BIT);
                    glPolygonMode( GL_FRONT, GL_FILL );
                    glEnableClientState(GL_VERTEX_ARRAY);
                    if (mIter->_hasMeshVertexColor)
                        glEnableClientState(GL_COLOR_ARRAY);
                    glVertexPointer(3,GL_FLOAT,0,&(mIter->_meshPositions[0]));
                    if (mIter->_hasMeshVertexColor)
                        glColorPointer(3,GL_FLOAT,0,&(mIter->_meshColors[0]));
                    if (mIter->_hasMeshVertexNormal)
                        glNormalPointer(GL_FLOAT,0,&(mIter->_meshNormals[0]));
                    glDrawElements(GL_TRIANGLES,mIter->_meshGLIndices.size(),GL_UNSIGNED_INT,
                                   &(mIter->_meshGLIndices[0]));
                    if (mIter->_hasMeshVertexColor)
                        glDisableClientState(GL_COLOR_ARRAY);
                    glDisableClientState(GL_VERTEX_ARRAY);
                    glPopAttrib();
                }
                view.endGL();
            }

        }
        break;
    default:
        break;
    }
}

bool BifrostSurfaceShape::isBounded() const
{
    return true;
}

MBoundingBox BifrostSurfaceShape::boundingBox() const
{
    if (_bm.size() != 0)
    {
        MBoundingBox bbox;
        BodyMeshDataCollection::const_iterator mEiter = _bm.end();
        BodyMeshDataCollection::const_iterator mIter  = _bm.begin();
        for (;mIter!=mEiter;++mIter)
        {
            bbox.expand(mIter->_meshBBox);
        }
        return bbox;
    }
    else if (_bf.size() != 0)
    {
        MBoundingBox bbox;
        BodyFieldDataCollection::const_iterator fEiter = _bf.end();
        BodyFieldDataCollection::const_iterator fIter  = _bf.begin();
        for (;fIter!=fEiter;++fIter)
        {
            bbox.expand(fIter->_fieldBBox);
        }
        return bbox;
    }
    else if (_hasParticleData)
        return _particleBBox;
    else
        return MBoundingBox(MPoint(-1,-1,-1),MPoint(1,1,1));
}

void BifrostSurfaceShape::setChannelNamesList(const MStringArray& attrList)
{
    MString tmp;
    int len = attrList.length();

    fAttributeListArray.clear();

    for (int i = 0; i < len; ++i)
    {
        tmp = attrList[i];
        fAttributeListArray.append(tmp);
    }
}

void BifrostSurfaceShape::drawBBox(const MBoundingBox& bbox) const
{
    glBegin( GL_LINE_LOOP );
    glVertex3f(bbox.max().x, bbox.max().y, bbox.max().z);
    glVertex3f(bbox.max().x, bbox.min().y, bbox.max().z);
    glVertex3f(bbox.max().x, bbox.min().y, bbox.min().z);
    glVertex3f(bbox.max().x, bbox.max().y, bbox.min().z);
    glEnd();
	
    glBegin( GL_LINE_LOOP );
    glVertex3f(bbox.min().x, bbox.max().y, bbox.max().z);
    glVertex3f(bbox.min().x, bbox.max().y, bbox.min().z);
    glVertex3f(bbox.min().x, bbox.min().y, bbox.min().z);
    glVertex3f(bbox.min().x, bbox.min().y, bbox.max().z);
    glEnd();
	
    glBegin( GL_LINES );
    glVertex3f(bbox.max().x, bbox.max().y, bbox.max().z);
    glVertex3f(bbox.min().x, bbox.max().y, bbox.max().z);
    //
    glVertex3f(bbox.max().x, bbox.min().y, bbox.max().z);
    glVertex3f(bbox.min().x, bbox.min().y, bbox.max().z);
    //
    glVertex3f(bbox.max().x, bbox.max().y, bbox.min().z);
    glVertex3f(bbox.min().x, bbox.max().y, bbox.min().z);
    //
    glVertex3f(bbox.max().x, bbox.min().y, bbox.min().z);
    glVertex3f(bbox.min().x, bbox.min().y, bbox.min().z);
    glEnd();
}

/*!
 * \note If we ever need to refactor *SurfaceShape
 * to an abstract class for reuse across different
 * file format Bifrost, ICE, PRT, OM, this is the method
 * which needs per format implementation
 */
MStatus BifrostSurfaceShape::compute( const MPlug& plug, MDataBlock& data )
{
    char debugStringBuf[BUFSIZ];

    // MGlobal::displayInfo("BifrostSurfaceShape::compute");
    MStatus status = MStatus::kSuccess;

    // See the original EMPSurfaceShape for the content,
    // For prototyping Bifrost still in Maya beta, everything is removed
    return status;
}

void BifrostSurfaceShape::update()
{
    // Check if the input file have change
    MObject thisNode = thisMObject();
    MPlug plug( thisNode, _outBoundingBoxAttr );

    MObject pointArrayData;
    plug.getValue( pointArrayData );
}


void BifrostSurfaceShape::accumulateMotionData()
{
}

void BifrostSurfaceShape::clearMotionData()
{
}

bool BifrostSurfaceShape::hasGeometryData() const
{
    return ( _bm.size() != 0 || _bp.size() != 0 );
}

bool BifrostSurfaceShape::hasFieldData() const
{
    return (_bf.size() != 0);
}

void* BifrostSurfaceShape::creator()
{
    return new BifrostSurfaceShape();
}

MStatus BifrostSurfaceShape::initialize()
{
    MFnUnitAttribute uAttr;
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    MFnMatrixAttribute mAttr;
    MFnStringData stringData;
    MFnStringArrayData stringArrayData;
    MStatus status = MStatus::kSuccess;

    // Input Bifrost filename attribute
    CMS(_inBifrostFileAttr = tAttr.create("BifrostFile", "pf",
                                          MFnData::kString, MObject::kNullObj,
                                          &status));
    CMS(status = addAttribute( _inBifrostFileAttr ));

    // Input time
    CMS(_inTimeAttr = uAttr.create( "time", "tm", MFnUnitAttribute::kTime, 0.0, &status ));
    CMS(status = uAttr.setStorable(true));
    CMS(status = uAttr.setWritable(true));
    CMS(status = addAttribute( _inTimeAttr ));
	
    /*
    // Input point width for RIB
    CMS(_inParticleWidthAttr = nAttr.create( "width", "wd", MFnNumericData::kDouble, 0.01, &status ));
    CMS(status = nAttr.setKeyable(true));
    CMS(status = addAttribute( _inParticleWidthAttr ));
    */

    // Input render velocity motion blur
    CMS(_inVelocityMotionBlurAttr = nAttr.create( "velocityBlur", "vb", MFnNumericData::kBoolean, false, & status));
    CMS(status = nAttr.setKeyable(true));
    CMS(status = addAttribute( _inVelocityMotionBlurAttr ));

    // Input chunk size for geometry output
    CMS(_inGeometryChunkAttr = nAttr.create( "chunk", "cn", MFnNumericData::kLong, 10000, &status ));
    CMS(status = nAttr.setKeyable(true));
    CMS(status = addAttribute( _inGeometryChunkAttr ));

    // Output channel names attribute
    // CMS(_outChannelNamesAttr = tAttr.create( "outChannelNames", "ocn", MFnData::kString ,stringData.create(MString("")), &status));
    CMS(_outChannelNamesAttr = tAttr.create("outChannelNames", "ocn", MFnData::kStringArray,
                                            stringArrayData.create(MStringArray()), &status));
    CMS(status = tAttr.setStorable( false ));
    CMS(status = tAttr.setKeyable( false ));
    CMS(status = tAttr.setArray( false ));
    CMS(status = tAttr.setWritable(false));
    CMS(status = tAttr.setReadable(true));
    CMS(status = tAttr.setHidden(true));
    CMS(status = addAttribute( _outChannelNamesAttr ));

    // Output bounding box attribute
    CMS(_outBoundingBoxAttr = tAttr.create("outBoundingBox", "obb", MFnPointArrayData::kPointArray,
                                           &status));
    CMS(status = addAttribute( _outBoundingBoxAttr ));

    // =========================
    // Attribute affector/affectee relationships
    CMS(status = attributeAffects( _inBifrostFileAttr, _outChannelNamesAttr ));
    CMS(status = attributeAffects( _inBifrostFileAttr, _outBoundingBoxAttr ));
    CMS(status = attributeAffects( _inTimeAttr, _outBoundingBoxAttr ));

    return status;
}
