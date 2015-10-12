#pragma once
#include <string>
#include <stdlib.h>
#include <utils/BifrostUtils.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <strstream>
#include <stdexcept>
#include <OpenEXR/ImathBox.h>

// Alembic headers - START
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/Util/All.h>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
// Alembic headers - END

#include <BifrostHeaders.h>

/*!
 * \brief Class to hold the translator state when extracting data from Bifrost
 *        file for export to Alembic
 * \note Position and velocity parameters are standard so not added as geom_param
 */
class Bifrost2Alembic
{
    typedef boost::shared_ptr<void> GeomParmPtr;
    GeomParmPtr density_geom_param;   // scalar
    GeomParmPtr vorticity_geom_param; // vector
    GeomParmPtr droplet_geom_param;   //scalar
    static Alembic::AbcGeom::GeometryScope _geometry_parameter_scope;
public:
	Bifrost2Alembic(const std::string& bifrost_filename,
					const std::string& alembic_filename,
					const std::string& position_channel_name,
					const std::string& velocity_channel_name,
					const std::string& density_channel_name,
					const std::string& vorticity_channel_name,
					const std::string& droplet_channel_name);
	virtual ~Bifrost2Alembic();
	bool translate();
protected:

	template <class O_GEOM_PARAM>
    void AddPointAttributes(uint32_t i_tsidx,
    						Alembic::AbcGeom::GeometryScope& i_geometry_parameter_scope,
							const std::string& i_geom_param_name,
							Alembic::AbcGeom::OPointsSchema &o_pSchema,
							GeomParmPtr& o_geom_parm)
    {
        Alembic::AbcGeom::MetaData mdata;
        Alembic::AbcGeom::SetGeometryScope(mdata, i_geometry_parameter_scope);

        Alembic::AbcGeom::OCompoundProperty arbParams = o_pSchema.getArbGeomParams();
        const bool is_param_indexed = false;
        o_geom_parm.reset(new O_GEOM_PARAM ( arbParams, i_geom_param_name.c_str(), is_param_indexed, i_geometry_parameter_scope, 1, i_tsidx ));

    }

	template <class O_GEOM_PARAM>
    void SetPointFloatAttributesData(const float* data,
    								 size_t data_size,
									 Alembic::AbcGeom::GeometryScope& i_geometry_parameter_scope,
									 GeomParmPtr& o_geom_parm)
    {
        if (o_geom_parm.get())
        {
            Alembic::AbcGeom::FloatArraySample array_sample( data, data_size );
            Alembic::AbcGeom::OFloatGeomParam::Sample dataSamp( array_sample, i_geometry_parameter_scope );

            boost::static_pointer_cast<O_GEOM_PARAM>(o_geom_parm)->set(dataSamp);
        }
    }

	Alembic::AbcGeom::OXform addXform(Alembic::Abc::OObject parent,
									  std::string name);

	bool process_point_component(bool is_bifrost_liquid_file,
								 const Bifrost::API::Component& component,
								 const std::string& position_channel_name,
								 const std::string& velocity_channel_name,
								 const std::string& density_channel_name,
								 const std::string& vorticity_channel_name,
								 const std::string& droplet_channel_name,
								 Imath::Box3f& bounds,
								 uint32_t tsidx,
								 Alembic::AbcGeom::OXform& xform);
private:
	std::string _bifrost_filename;
	std::string _alembic_filename;
	std::string _position_channel_name;
	std::string _velocity_channel_name;
	std::string _density_channel_name;
	std::string _vorticity_channel_name;
	std::string _droplet_channel_name;
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
