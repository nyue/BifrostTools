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
#include <boost/format.hpp>

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
, _BifrostFilePathChanged(false)
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
	MGlobal::displayInfo("boundingBox() called");
	if (_bm.size() != 0)
	{
		MBoundingBox bbox;
		BodyMeshDataCollection::const_iterator mEiter = _bm.end();
		BodyMeshDataCollection::const_iterator mIter  = _bm.begin();
		for (;mIter!=mEiter;++mIter)
		{
			bbox.expand(mIter->_meshBBox);
		}
		MGlobal::displayInfo("boundingBox() return bbox[body mesh]");
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
		MGlobal::displayInfo("boundingBox() return bbox[body field]");
		return bbox;
	}
	else if (_hasParticleData)
	{
		MGlobal::displayInfo("boundingBox() return bbox[particle]");
		return _particleBBox;
	}
	else
	{
		MGlobal::displayInfo("boundingBox() return bbox[default unit]");
		return MBoundingBox(MPoint(-1,-1,-1),MPoint(1,1,1));
	}
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

template<typename T>
bool BifrostSurfaceShape::processChannelData(Bifrost::API::Component& component,
		Bifrost::API::Channel& channel_data,
		bool i_is_point_position,
		std::vector<T>& o_channel_data_array) const
{
	Bifrost::API::Layout layout = component.layout();
	float voxel_scale = layout.voxelScale();
	size_t depthCount = layout.depthCount();


	for ( size_t d=0; d<depthCount; d++ ) {
		size_t tcount = layout.tileCount(d);
		for ( size_t t=0; t<tcount; t++ ) {
			Bifrost::API::TreeIndex tindex(t,d);
			if ( !channel_data.elementCount( tindex ) ) {
				// nothing there
				continue;
			}
			Bifrost::API::TileData<T> data_element = channel_data.tileData<T>( tindex );
			for (size_t i=0; i<data_element.count(); i++ ) {
				if (i_is_point_position)
					o_channel_data_array.push_back(data_element[i] * voxel_scale);
				else
					o_channel_data_array.push_back(data_element[i]);
			}
		}
	}
	return true;
}

bool BifrostSurfaceShape::loadParticleData(const MString& i_bifrost_filename,
										   GLfloatVector& o_particlePositions,
										   GLuintVector&  o_particleGLIndices)
{
	o_particlePositions.clear();
	o_particleGLIndices.clear();

	Bifrost::API::String biffile = i_bifrost_filename.asChar();

	Bifrost::API::ObjectModel om;
	Bifrost::API::FileIO fileio = om.createFileIO( biffile );
	Bifrost::API::StateServer ss = fileio.load( );
	const Bifrost::API::BIF::FileInfo& info = fileio.info();

	if ( !ss.valid() ) {
		std::cerr << boost::format("Unable to load the content of the Bifrost file \"%1%\"") % biffile.c_str()
                				  << std::endl;
		return false;
	}

	Bifrost::API::Component component = ss.components()[0];
	if ( component.type() != Bifrost::API::PointComponentType ) {
		std::cerr << "Wrong component (" << component.type() << ")" << std::endl;
		return false;
	}

	// float bifrost_scale = 0.0;

	Bifrost::API::RefArray channels = component.channels();

	for (size_t channelIndex=0;channelIndex<info.channelCount;channelIndex++)
	{
		const Bifrost::API::BIF::FileInfo::ChannelInfo& channelInfo = fileio.channelInfo(channelIndex);
		std::cout << boost::format("channelInfo.name = %1%") % channelInfo.name.c_str() << std::endl;
		bool is_point_position = channelInfo.name.find("position") != Bifrost::API::String::npos;

		if (is_point_position)
		{
			std::cout << boost::format("FOUND position = %1%") % channelInfo.name.c_str() << std::endl;
			switch (channelInfo.dataType)
			{
			case		Bifrost::API::FloatV3Type:	/*!< Defines a channel of type amino::Math::vec3f. #3 */
			{
				std::cout << "FOUND suitable FloatV3Type"<< std::endl;
				typedef amino::Math::vec3f DataType;
				std::vector<DataType> channel_data_array;
				Bifrost::API::Channel channel = channels[channelIndex];

				bool successfully_processed = processChannelData<DataType>(component,channel,is_point_position,channel_data_array);
				if (successfully_processed)
				{
					size_t numParticles = channel_data_array.size();
					std::cout << boost::format("SUCCESSFULLY processed %1% points") % numParticles << std::endl;
					//								for (size_t i = 0; i<numParticles;i++)
					//									v3_array.array()[i].assign(channel_data_array[i].v[0],channel_data_array[i].v[1],channel_data_array[i].v[2]);
					for (size_t i = 0; i<numParticles;i++)
					{
						o_particlePositions.push_back(channel_data_array[i].v[0]);
						o_particlePositions.push_back(channel_data_array[i].v[1]);
						o_particlePositions.push_back(channel_data_array[i].v[2]);
						_particleBBox.expand(MPoint(channel_data_array[i].v[0],channel_data_array[i].v[1],channel_data_array[i].v[2]));
						o_particleGLIndices.push_back(i);
					}
					_hasParticleData = true;

				}
				else
				{
					// Return early, no point processing the other attribute if position is not found
					return false;
				}
			}
			break;
			default:
				break;
			}
		}
	}
	return true;
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
MStatus BifrostSurfaceShape::compute( const MPlug& plug, MDataBlock& data_block )
{
	MGlobal::displayInfo("compute() called");
	char debugStringBuf[BUFSIZ];

	// MGlobal::displayInfo("BifrostSurfaceShape::compute");
	MStatus status = MStatus::kSuccess;

	// if ( !data_block.isClean( _inBifrostFileAttr ) )
	{
		CMS(MDataHandle inputPathHdl = data_block.inputValue( _inBifrostFileAttr, &status ));
		MString bifrostFilePath = inputPathHdl.asString();
		if (bifrostFilePath.length()>0 && _BifrostFilePath != bifrostFilePath)
		{
			MGlobal::displayInfo((boost::format("compute() NEW _inBifrostFileAttr = '%1%'") % bifrostFilePath.asChar()).str().c_str());
			_BifrostFilePathChanged = true;
			_BifrostFilePath = bifrostFilePath;

			if (!loadParticleData(_BifrostFilePath,_particlePositions,_particleGLIndices))
				return MStatus::kFailure;
		}
	}

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
