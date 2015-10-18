#pragma once

#include <iostream>
#include <maya/MPxSurfaceShape.h>
#include <maya/MStatus.h>
#include <maya/M3dView.h>
#include <maya/MStringArray.h>
#include <vector>

class BifrostSurfaceShape : public MPxSurfaceShape
{
	typedef std::vector<GLfloat> GLfloatVector;
	typedef std::vector<GLuint> GLuintVector;
	struct BodyMeshData {
		MBoundingBox _meshBBox;
		GLfloatVector _meshPositions;
		GLfloatVector _meshColors;
		GLfloatVector _meshNormals;
		GLuintVector _meshGLIndices;
		GLuintVector _meshVertexGLIndices;
		bool _hasMeshVertexColor;
		bool _hasMeshVertexNormal;
	};
	struct BodyParticleData {
		GLfloatVector _particlePositions;
		GLfloatVector _particleColors;
		GLuintVector _particleGLIndices;
	};
	struct BodyFieldData {
		MBoundingBox _fieldBBox;
	};
	typedef std::vector<BodyMeshData> BodyMeshDataCollection;
	typedef std::vector<BodyParticleData> BodyParticleDataCollection;
	typedef std::vector<BodyFieldData> BodyFieldDataCollection;
public:
	BifrostSurfaceShape();
	virtual ~BifrostSurfaceShape();

	// MPxSurfaceShape methods to over-ride (from Maya)
	virtual void postConstructor();
    virtual void draw (M3dView& view,
                       const MDagPath& path,
                       M3dView::DisplayStyle style,
                       M3dView::DisplayStatus status);
    virtual bool            isBounded() const;
    virtual MBoundingBox    boundingBox() const;
 	virtual MStatus compute( const MPlug& plug, MDataBlock& data );

 	/*! \brief use by BifrostSurfaceShapeUI to update node */
 	void update();

	/*! \brief Accumulate motion block data during 3Delight -addstep call */
	void accumulateMotionData();
	/*! \brief Clears the accumulated motion data to free up the memory */
	void clearMotionData();

	/*! \brief Do we have body geometry data */
	bool hasGeometryData() const;
	/*! \brief Do we have body field data */
	bool hasFieldData() const;

	// Parent class method to implement
	static void *  creator();
	static MStatus initialize();
	static MTypeId typeId;
private:
	void setChannelNamesList(const MStringArray& attrList);

// 	void processParticleData(const Nb::Body*     body,
// 							 const Nb::String&   channels);
// 	void processMeshData(const Nb::Body*         body,
// 						 const Nb::String&       channels,
// 						 BodyMeshDataCollection& bm);
// 	void processFieldData(const Nb::Body*     body,
// 						  const Nb::String&   channels,
// 						  BodyFieldDataCollection& bf);
    void drawBBox(const MBoundingBox& bbox) const;
	MBoundingBox _particleBBox;
	static MObject _inBifrostFileAttr;
	static MObject _inTimeAttr;
	// static MObject _inParticleWidthAttr;
	static MObject _inVelocityMotionBlurAttr;
	static MObject _inGeometryChunkAttr;
	static MObject _outChannelNamesAttr;
	static MObject _outBoundingBoxAttr;
	MStringArray   fAttributeListArray;
	GLfloatVector _particlePositions;
	GLfloatVector _particleColors;
	GLuintVector _particleGLIndices;

	MString _BifrostFilePath;
	bool _hasParticleColor;
	bool _hasParticleData;

	BodyMeshDataCollection _bm;
	BodyParticleDataCollection _bp;
	BodyFieldDataCollection _bf;
};
// == Emacs ================
// -------------------------
// Local variables:
// tab-width: 4
// indent-tabs-mode: t
// c-basic-offset: 4
// end:
//
// == vi ===================
// -------------------------
// Format block
// ex:ts=4:sw=4:expandtab
// -------------------------
