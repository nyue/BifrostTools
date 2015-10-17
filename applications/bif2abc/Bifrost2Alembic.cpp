#include "Bifrost2Alembic.h"

Alembic::AbcGeom::GeometryScope Bifrost2Alembic::_geometry_parameter_scope = Alembic::AbcGeom::kVaryingScope;

Bifrost2Alembic::Bifrost2Alembic(const std::string& bifrost_filename,
								 const std::string& alembic_filename,
								 const std::string& position_channel_name,
								 const std::string& velocity_channel_name,
								 const std::string& density_channel_name,
								 const std::string& vorticity_channel_name,
								 const std::string& droplet_channel_name)
: _bifrost_filename(bifrost_filename)
, _alembic_filename(alembic_filename)
, _position_channel_name(position_channel_name)
, _velocity_channel_name(velocity_channel_name)
, _density_channel_name(density_channel_name)
, _vorticity_channel_name(vorticity_channel_name)
, _droplet_channel_name(droplet_channel_name)
{

}

Bifrost2Alembic::~Bifrost2Alembic()
{

}

bool Bifrost2Alembic::translate()
{

    Bifrost::API::String biffile = _bifrost_filename.c_str();
    Bifrost::API::ObjectModel om;
    Bifrost::API::FileIO fileio = om.createFileIO( biffile );
    const Bifrost::API::BIF::FileInfo& info = fileio.info();
    bool is_bifrost_liquid_file(true);

    // Need to determine is the BIF file contains Foam or Liquid particle info
    if (std::string(info.componentName.c_str()).find("Foam")!=std::string::npos)
    	is_bifrost_liquid_file = false;

    // Need to load the entire file's content to process
    Bifrost::API::StateServer ss = fileio.load( );
    if (ss.valid())
    {
        size_t numComponents = ss.components().count();
        if (numComponents>0)
        {
            /*
             * Create the Alembic file only if we get a valid state server
             * after loading and there is at least one component
             */

            Alembic::AbcGeom::OArchive archive(Alembic::Abc::CreateArchiveWithInfo(
            		// Alembic::AbcCoreHDF5::WriteArchive(),
            		Alembic::AbcCoreOgawa::WriteArchive(),
                                                                                   _alembic_filename.c_str(),
                                                                                   std::string("Procedural Insight Pty. Ltd."),
                                                                                   std::string("info@proceduralinsight.com")));
            Alembic::AbcGeom::OObject topObj( archive, Alembic::AbcGeom::kTop );
            Alembic::AbcGeom::OXform xform = addXform(topObj,"bif2abc");

            // Create the time sampling
            Alembic::Abc::chrono_t fps = 24.0;
            Alembic::Abc::chrono_t startTime = 0.0;
            Alembic::Abc::chrono_t iFps = 1.0/fps;
            Alembic::Abc::TimeSampling ts(iFps,startTime);
            uint32_t tsidx = topObj.getArchive().addTimeSampling(ts);

            for (size_t componentIndex=0;componentIndex<numComponents;componentIndex++)
            {
                Bifrost::API::Component component = ss.components()[componentIndex];
                Bifrost::API::TypeID componentType = component.type();
                if (componentType == Bifrost::API::PointComponentType)
                {
                    Imath::Box3f bounds;
                    process_point_component(is_bifrost_liquid_file,
                    						component,
											_position_channel_name,
											_velocity_channel_name,
											_density_channel_name,
											_vorticity_channel_name,
											_droplet_channel_name,
											bounds,
											tsidx,
											xform);
                }
            }
        }
    }
    else
    {
        std::cerr << boost::format("Unable to load the content of the Bifrost file \"%1%\"") % _bifrost_filename.c_str()
                  << std::endl;
        return false;
    }
    return true;

}

Alembic::AbcGeom::OXform
Bifrost2Alembic::addXform(Alembic::Abc::OObject parent, std::string name)
{
    Alembic::AbcGeom::OXform xform(parent, name.c_str());

    return xform;
}

bool Bifrost2Alembic::process_point_component(bool is_bifrost_liquid_file,
											  const Bifrost::API::Component& component,
											  const std::string& position_channel_name,
											  const std::string& velocity_channel_name,
											  const std::string& density_channel_name,
											  const std::string& vorticity_channel_name,
											  const std::string& droplet_channel_name,
											  Imath::Box3f& bounds,
											  uint32_t tsidx,
											  Alembic::AbcGeom::OXform& xform)
{
	// All channel variables (not all will be initialized)
    Bifrost::API::Channel position_ch;  // in both liquid and foam
    Bifrost::API::Channel velocity_ch;  // in both liquid and foam
    Bifrost::API::Channel density_ch;   // in both liquid and foam
    Bifrost::API::Channel vorticity_ch; // only in liquid
    Bifrost::API::Channel droplet_ch;   // only in liquid

    // Position channel
    bool position_channel_status = false;
    get_channel(component,position_channel_name,Bifrost::API::FloatV3Type,position_ch,position_channel_status);
    if (!position_channel_status)
    {
    	return false;
    }

    // Density channel
    bool density_channel_status = false;
    get_channel(component,density_channel_name,Bifrost::API::FloatType,density_ch,density_channel_status);
    if (!density_channel_status)
    {
    	return false;
    }

    // Velocity channel
    bool velocity_channel_status = false;
    get_channel(component,velocity_channel_name,Bifrost::API::FloatV3Type,velocity_ch,velocity_channel_status);
    if (!velocity_channel_status)
    {
    	return false;
    }

    /*!
     * \note Additional attributes requiring handling
     * \li Vorticity (vector)
     * \li Droplet (scalar)
     */
    if (is_bifrost_liquid_file)
    {
        // Vorticity channel
        bool vorticity_channel_status = false;
        get_channel(component,vorticity_channel_name,Bifrost::API::FloatType,vorticity_ch,vorticity_channel_status);
        if (!vorticity_channel_status)
        {
        	return false;
        }

        // Droplet channel
        bool droplet_channel_status = false;
        get_channel(component,droplet_channel_name,Bifrost::API::FloatType,droplet_ch,droplet_channel_status);
        if (!droplet_channel_status)
        {
        	return false;
        }

    }
    // Create the OPoints object
    Alembic::AbcGeom::OPoints partsOut(xform,component.name().c_str(),tsidx);
    Alembic::AbcGeom::OPointsSchema &pSchema = partsOut.getSchema();

    Alembic::AbcGeom::MetaData mdata;
    SetGeometryScope( mdata, Alembic::AbcGeom::kVaryingScope );
    Alembic::AbcGeom::OV3fArrayProperty velOut( pSchema, ".velocities", mdata, tsidx );

    std::vector< Alembic::Abc::V3f > positions;
    std::vector< Alembic::Abc::V3f > velocities;
    std::vector< float > densities;
    std::vector< float > vorticities;
    std::vector< float > droplets;
    std::vector< Alembic::Util::uint64_t > ids;

    // NOTE : Other than position, velocity and id, all the other information
    //        are expected to be store as arbGeomParam
    AddPointAttributes<Alembic::AbcGeom::OFloatGeomParam>(tsidx,
    													  _geometry_parameter_scope,
														  "density",
														  pSchema,
														  density_geom_param);
    if (is_bifrost_liquid_file)
    {
        AddPointAttributes<Alembic::AbcGeom::OFloatGeomParam>(tsidx,
        													  _geometry_parameter_scope,
    														  "vorticity",
    														  pSchema,
    														  vorticity_geom_param);
        AddPointAttributes<Alembic::AbcGeom::OFloatGeomParam>(tsidx,
        													  _geometry_parameter_scope,
    														  "droplet",
    														  pSchema,
    														  droplet_geom_param);
    }
    // Data accumulation
    Bifrost::API::Layout layout = component.layout();
    size_t depthCount = layout.depthCount();
    Alembic::Util::uint64_t currentId = 0;
    for ( size_t d=0; d<depthCount; d++ ) {
        for ( size_t t=0; t<layout.tileCount(d); t++ ) {
            Bifrost::API::TreeIndex tindex(t,d);
            if ( !position_ch.elementCount( tindex ) ) {
                // nothing there
                continue;
            }

            float _MVS = layout.voxelScale();
            // std::cerr << boost::format("_MVS = %1%") % _MVS << std::endl;
            const Bifrost::API::TileData<amino::Math::vec3f>& position_tile_data = position_ch.tileData<amino::Math::vec3f>( tindex );
            const Bifrost::API::TileData<amino::Math::vec3f>& velocity_tile_data = velocity_ch.tileData<amino::Math::vec3f>( tindex );
            const Bifrost::API::TileData<float>& density_tile_data = density_ch.tileData<float>( tindex );
            if (position_tile_data.count() != velocity_tile_data.count())
            {
                std::cerr << boost::format("Point position and velocity tile data count mismatch position[%1%] velocity[%2%]") % position_tile_data.count() % velocity_tile_data.count()<< std::endl;
                return false;
            }
            // Position
            for (size_t i=0; i<position_tile_data.count(); i++ )
            {
                Imath::V3f point_position(position_tile_data[i][0]*_MVS,
										  position_tile_data[i][1]*_MVS,
										  position_tile_data[i][2]*_MVS);
                positions.push_back(point_position);
                bounds.extendBy(point_position);
                ids.push_back(currentId);
                currentId++;
            }
            // Velocity
            for (size_t i=0; i<velocity_tile_data.count(); i++ )
            {
                velocities.push_back(Imath::V3f (velocity_tile_data[i][0],
												 velocity_tile_data[i][1],
												 velocity_tile_data[i][2]));
            }
            // Density
            for (size_t i=0; i<density_tile_data.count(); i++ )
            {
            	densities.push_back(density_tile_data[i]);
            }

            if (is_bifrost_liquid_file)
            {
                const Bifrost::API::TileData<float>& vorticity_tile_data = vorticity_ch.tileData<float>( tindex );
                const Bifrost::API::TileData<float>& droplet_tile_data = droplet_ch.tileData<float>( tindex );
            	// Vorticity
                for (size_t i=0; i<vorticity_tile_data.count(); i++ )
                {
                	vorticities.push_back(vorticity_tile_data[i]);
                }

            	// Droplet
                for (size_t i=0; i<droplet_tile_data.count(); i++ )
                {
                	droplets.push_back(droplet_tile_data[i]);
                }
            }
        }
    }

    // Update Alembic storage
    Alembic::AbcGeom::V3fArraySample position_data ( positions );
    Alembic::AbcGeom::UInt64ArraySample id_data ( ids );
    Alembic::AbcGeom::OPointsSchema::Sample psamp(position_data,
												  id_data);
    pSchema.set( psamp );
    velOut.set( Alembic::AbcGeom::V3fArraySample( velocities ) );
    // Geometry Parameters handling
    SetPointFloatAttributesData<Alembic::AbcGeom::OFloatGeomParam>(densities.data(),
    															   densities.size(),
																   _geometry_parameter_scope,
																   density_geom_param);
    if (is_bifrost_liquid_file)
    {
    	// Vorticity
        SetPointFloatAttributesData<Alembic::AbcGeom::OFloatGeomParam>(vorticities.data(),
        															   vorticities.size(),
    																   _geometry_parameter_scope,
    																   vorticity_geom_param);
    	// Droplet
        SetPointFloatAttributesData<Alembic::AbcGeom::OFloatGeomParam>(droplets.data(),
        															   droplets.size(),
    																   _geometry_parameter_scope,
    																   droplet_geom_param);
    }
	return true;
}



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
