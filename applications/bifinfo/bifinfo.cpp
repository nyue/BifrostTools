#include <utils/BifrostUtils.h>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <OpenEXR/ImathBox.h>

#include <BifrostHeaders.h>

typedef std::vector<Bifrost::API::String> StringContainer;

namespace po = boost::program_options;

// A helper function to simplify the main part.
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    return os;
}

enum BBOX { None, PointsOnly, PointsWithVelocity };

inline std::ostream& operator<<(std::ostream & os, BBOX & bbox)
{
    switch (bbox) {
    case None:
        os << "None";
        break;
    case PointsOnly:
        os << "Points only";
        break;
    case PointsWithVelocity:
        os << "Points with velocity";
    }
    return os;
}

inline std::istream & operator>>(std::istream & str, BBOX & bbox) {
    unsigned int bbox_type = 0;
    if (str >> bbox_type)
        bbox = static_cast<BBOX>(bbox_type);
    return str;
}

/*!
 * \brief Output to a string stream makes it easier to
 *        accommodate both use case of file and console
 */
void bounds_as_wavefront_strstream(const Imath::Box3f& bounds,
                                   std::ostream& os)
{

    os << "# Procedural Insight Pty. Ltd. 2015" << std::endl;
    os << "# www.proceduralinsight.com" << std::endl;
    os << boost::format("v %1% %2% %3%") % bounds.min.x % bounds.min.y % bounds.min.z << std::endl;
    os << boost::format("v %1% %2% %3%") % bounds.min.x % bounds.min.y % bounds.max.z << std::endl;
    os << boost::format("v %1% %2% %3%") % bounds.max.x % bounds.min.y % bounds.max.z << std::endl;
    os << boost::format("v %1% %2% %3%") % bounds.max.x % bounds.min.y % bounds.min.z << std::endl;
    os << boost::format("v %1% %2% %3%") % bounds.min.x % bounds.max.y % bounds.min.z << std::endl;
    os << boost::format("v %1% %2% %3%") % bounds.min.x % bounds.max.y % bounds.max.z << std::endl;
    os << boost::format("v %1% %2% %3%") % bounds.max.x % bounds.max.y % bounds.max.z << std::endl;
    os << boost::format("v %1% %2% %3%") % bounds.max.x % bounds.max.y % bounds.min.z << std::endl;
    os << "f 1 2 6 5" << std::endl;
    os << "f 4 8 7 3" << std::endl;
    os << "f 5 6 7 8" << std::endl;
    os << "f 1 4 3 2" << std::endl;
    os << "f 7 6 2 3" << std::endl;
    os << "f 5 8 4 1" << std::endl;

}

void process_bounds(const std::string& label,const Imath::Box3f& bounds)
{
    std::cout << label << std::endl;
    std::cout << "\t min [" << bounds.min << "] max[" << bounds.max << "]"<< std::endl;
}

void process_bounds_as_renderman(const std::string& label,const Imath::Box3f& bounds)
{
    std::cout << label << std::endl;
    std::cout << boost::format("\t RiBound [ %1% %2% %3% %4% %5% %6%]")
        % bounds.min.x
        % bounds.max.x
        % bounds.min.y
        % bounds.max.y
        % bounds.min.z
        % bounds.max.z
              << std::endl;
}

int determine_points_bbox(const Bifrost::API::Component& component,
                          const std::string& position_channel_name,
                          Imath::Box3f& bounds)
{
    int positionChannelIndex = findChannelIndexViaName(component,position_channel_name.c_str());
    if (positionChannelIndex<0)
        return 1;
    const Bifrost::API::Channel& position_ch = component.channels()[positionChannelIndex];
    if (!position_ch.valid())
        return 1;
    if ( position_ch.dataType() != Bifrost::API::FloatV3Type)
        return 1;

    Bifrost::API::Layout layout = component.layout();
    size_t depthCount = layout.depthCount();
    for ( size_t d=0; d<depthCount; d++ ) {
        for ( size_t t=0; t<layout.tileCount(d); t++ ) {
            Bifrost::API::TreeIndex tindex(t,d);
            if ( !position_ch.elementCount( tindex ) ) {
                // nothing there
                continue;
            }

            const Bifrost::API::TileData<amino::Math::vec3f>& position_tile_data = position_ch.tileData<amino::Math::vec3f>( tindex );
            // const Bifrost::API::TileData<amino::Math::vec3f>& velocity_tile_data = velocity_ch.tileData<amino::Math::vec3f>( tindex );
            for (size_t i=0; i<position_tile_data.count(); i++ ) {
                bounds.extendBy(Imath::V3f(position_tile_data[i][0],
                                           position_tile_data[i][1],
                                           position_tile_data[i][2]));
            }
        }
    }
    return 0;
}

int determine_points_with_velocity_bbox(const Bifrost::API::Component& component,
                                        const std::string& position_channel_name,
                                        const std::string& velocity_channel_name,
                                        float fps,
                                        Imath::Box3f& bounds)
{
    int positionChannelIndex = findChannelIndexViaName(component,position_channel_name.c_str());
    if (positionChannelIndex<0)
        return 1;
    int velocityChannelIndex = findChannelIndexViaName(component,velocity_channel_name.c_str());
    if (velocityChannelIndex<0)
        return 1;
    const Bifrost::API::Channel& position_ch = component.channels()[positionChannelIndex];
    if (!position_ch.valid())
        return 1;
    const Bifrost::API::Channel& velocity_ch = component.channels()[velocityChannelIndex];
    if (!velocity_ch.valid())
        return 1;
    if ( position_ch.dataType() != Bifrost::API::FloatV3Type)
        return 1;
    if ( velocity_ch.dataType() != Bifrost::API::FloatV3Type)
        return 1;

    Bifrost::API::Layout layout = component.layout();
    size_t depthCount = layout.depthCount();
    float fps_1 = 1.0/fps;
    for ( size_t d=0; d<depthCount; d++ ) {
        for ( size_t t=0; t<layout.tileCount(d); t++ ) {
            Bifrost::API::TreeIndex tindex(t,d);
            if ( !position_ch.elementCount( tindex ) ) {
                // nothing there
                continue;
            }
            if (position_ch.elementCount( tindex ) != velocity_ch.elementCount( tindex ))
                return 1;
            const Bifrost::API::TileData<amino::Math::vec3f>& position_tile_data = position_ch.tileData<amino::Math::vec3f>( tindex );
            const Bifrost::API::TileData<amino::Math::vec3f>& velocity_tile_data = velocity_ch.tileData<amino::Math::vec3f>( tindex );
            for (size_t i=0; i<position_tile_data.count(); i++ ) {
                bounds.extendBy(Imath::V3f(position_tile_data[i][0],
                                           position_tile_data[i][1],
                                           position_tile_data[i][2]));
                bounds.extendBy(Imath::V3f(position_tile_data[i][0] + (fps_1 * velocity_tile_data[i][0]),
                                           position_tile_data[i][1] + (fps_1 * velocity_tile_data[i][1]),
                                           position_tile_data[i][2] + (fps_1 * velocity_tile_data[i][2])));
            }
        }
    }

    return 0;
}

void VoxelComponentTypeBBox(BBOX                           bbox_type,
                            const Bifrost::API::Component& component)
{

	{
	    Bifrost::Math::Similarity toWorld;

	    Bifrost::API::Dictionary dict = component.dictionary();

	    Bifrost::API::String dict_json = dict.saveJSON();
        std::cout << boost::format("Dictionary as JSON = '%1%'") % dict_json.c_str() << std::endl;


	    if(component.dictionary().hasValue("inv") && component.dictionary().hasValue("sim") )
	    {
	        toWorld.inv = component.dictionary().value<amino::Math::mat44f>("inv");
	        toWorld.sim = component.dictionary().value<amino::Math::mat44f>("sim");
	        std::cout << boost::format("Component inverse matrix = %1%") % toWorld.inv << std::endl;
	        std::cout << boost::format("Component simulation matrix = %1%") % toWorld.sim << std::endl;
	    }

	}

    Bifrost::API::Layout layout = component.layout();
	Bifrost::API::TileAccessor accessor = layout.tileAccessor();

    float voxel_scale = layout.voxelScale();
    std::string density_channel_name("density");

    int densityChannelIndex = findChannelIndexViaName(component,density_channel_name.c_str());
    if (densityChannelIndex<0)
        return;

    const Bifrost::API::Channel& density_ch = component.channels()[densityChannelIndex];
    if (!density_ch.valid())
        return;

    if ( density_ch.dataType() != Bifrost::API::FloatType)
    	return;

    size_t depthCount = layout.depthCount();


    if (true)
    {
    for ( size_t d=0; d<depthCount; d++ ) {
        for ( size_t t=0; t<layout.tileCount(d); t++ ) {
            Bifrost::API::TreeIndex tindex(t,d);
            if ( !density_ch.elementCount( tindex ) ) {
                // nothing there
                continue;
            }
            const Bifrost::API::TileData<float>& density_tile_data = density_ch.tileData<float>( tindex );
			const Bifrost::API::Tile tile = accessor.tile(tindex);
			const Bifrost::API::TileInfo tile_info = tile.info();
			const Bifrost::API::TileDimInfo tile_dim_info = tile_info.dimInfo;
			// tile.coord()
			size_t data_count = density_tile_data.count();
			std::cout << boost::format("data_count[d=%1%][t=%2%] = %3%") % d % t % data_count << std::endl;
			/*
            for (size_t i=0; i<density_tile_data.count(); i++ ) {
                std::cout << boost::format("Density[%1%][%2%][%3%] = %4%") % d % t % i % density_tile_data[i] << std::endl;
            }
			*/
        }
    }
    }
    std::cout << boost::format("Voxel scale        = %1%") % voxel_scale << std::endl;
    std::cout << boost::format("Layout depth count = %1%") % depthCount << std::endl;
}

void PointComponentTypeBBox(BBOX                           bbox_type,
                            const std::string&             position_channel_name,
                            const std::string&             velocity_channel_name,
                            const Bifrost::API::Component& component,
                            const float*                   fps)
{
    Imath::Box3f bounds;
    int bbox_status;
    switch(bbox_type)
    {
    case BBOX::PointsOnly :
        bbox_status = determine_points_bbox(component,
                                            position_channel_name,
                                            bounds);
        process_bounds("[NEW] PointComponentType : Points only",bounds);
        process_bounds_as_renderman("[NEW] PointComponentType : Points only",bounds);
        {
            std::stringstream ostream;
            bounds_as_wavefront_strstream(bounds, ostream);
            ostream << '\0';
            std::cout << ostream.str();
        }

        break;
    case BBOX::PointsWithVelocity :
        bbox_status = determine_points_with_velocity_bbox(component,
                                                          position_channel_name,
                                                          velocity_channel_name,
                                                          *fps,
                                                          bounds);
        process_bounds("[NEW] PointComponentType : Points with velocity",bounds);
        process_bounds_as_renderman("[NEW] PointComponentType : Points with velocity",bounds);
        break;
    default:
        break;
    }



}

void process_VoxelComponentType(const Bifrost::API::Component& component)
{
	{
		std::cout << "process_VoxelComponentType() START" << std::endl;
		Bifrost::API::Layout layout = component.layout();
		size_t depth_count = layout.depthCount();
		Bifrost::API::RefArray channel_array = component.channels();
		size_t num_channels = channel_array.count();
		std::cout << boost::format("process_VoxelComponentType() number of channels %1%") % num_channels << std::endl;
		for (size_t channel_index = 0; channel_index < num_channels; ++channel_index)
		{
			const Bifrost::API::Channel current_channel = channel_array[channel_index];
			size_t channel_element_count = current_channel.elementCount();
			std::cout << boost::format("process_VoxelComponentType() channel_element_count %1%") % channel_element_count << std::endl;
			for (size_t depth_index = 0; depth_index < depth_count; depth_index++) {
				Bifrost::API::TileDimInfo tile_dim_info = layout.tileDimInfo(depth_index);
				std::cout << boost::format("process_VoxelComponentType() tile_dim_info[%1%][%2%] tileSize = %3%, tileWidth = %4%, depthWidth = %5%, voxelWidth = %6%")
					% current_channel.name()
					% depth_index
					% tile_dim_info.tileSize
					% tile_dim_info.tileWidth
					% tile_dim_info.depthWidth
					% tile_dim_info.voxelWidth
					<< std::endl;
				size_t tile_count = layout.tileCount(depth_index);
				for (size_t tile_index = 0; tile_index < tile_count; tile_index++) {
					Bifrost::API::TreeIndex tree_index(tile_index, depth_index);
					switch (current_channel.dataType())
					{
					case Bifrost::API::FloatType : //,		/*!< Type float */
						// std::cout << "Channel type is FloatType" << std::endl;
					{
						
						const Bifrost::API::TileData<float>& channel_data = current_channel.tileData<float>(tree_index);
						size_t channel_data_count = channel_data.count();
						std::cout << boost::format("process_VoxelComponentType() channel[%1%] count = %2%") % current_channel.name() % channel_data_count << std::endl;
						for (size_t channel_data_index = 0; channel_data_index<channel_data_count; channel_data_index++) {
							// std::cout << boost::format("process_VoxelComponentType() channel[%1%][%2%] =  %3%") % current_channel.name() % channel_data_index % channel_data[channel_data_index] << std::endl;
						}
					}
						break;
					case Bifrost::API::FloatV2Type : //,	/*!< Type amino::Math::vec2f */
						// std::cout << "Channel type is FloatV2Type" << std::endl;
						break;
					case Bifrost::API::FloatV3Type : //,	/*!< Type amino::Math::vec3f */
						std::cout << "Channel type is FloatV3Type" << std::endl;
						break;
					case Bifrost::API::Int32Type : //,		/*!< Type int32_t */
						std::cout << "Channel type is Int32Type" << std::endl;
						break;
					case Bifrost::API::Int64Type : //,		/*!< Type int64_t */
						std::cout << "Channel type is Int64Type" << std::endl;
						break;
					case Bifrost::API::UInt32Type : //,		/*!< Type uint32_t */
						std::cout << "Channel type is UInt32Type" << std::endl;
						break;
					case Bifrost::API::UInt64Type : //,		/*!< Type uint64_t */
						std::cout << "Channel type is UInt64Type" << std::endl;
						break;
					case Bifrost::API::Int32V2Type : //,	/*!< Type amino::Math::vec2i */
						std::cout << "Channel type is Int32V2Type" << std::endl;
						break;
					case Bifrost::API::Int32V3Type : //,	/*!< Type amino::Math::vec3i */
						std::cout << "Channel type is Int32V3Type" << std::endl;
						break;
					case Bifrost::API::FloatV4Type : //,	/*!< Type amino::Math::vec4f */
						std::cout << "Channel type is FloatV4Type" << std::endl;
						break;
					case Bifrost::API::FloatMat44Type : //,	/*!< Type amino::Math::mat44f */
						std::cout << "Channel type is FloatMat44Type" << std::endl;
						break;
					case Bifrost::API::Int8Type : //,		/*!< Type int8_t */
						std::cout << "Channel type is Int8Type" << std::endl;
						break;
					case Bifrost::API::Int16Type : //,		/*!< Type int16_t */
						std::cout << "Channel type is Int16Type" << std::endl;
						break;
					case Bifrost::API::UInt8Type : //,		/*!< Type uint8_t */
						std::cout << "Channel type is UInt8Type" << std::endl;
						break;
					case Bifrost::API::UInt16Type : //,		/*!< Type uint16_t */
						std::cout << "Channel type is UInt16Type" << std::endl;
						break;
					case Bifrost::API::BoolType : //,		/*!< Type bool */
						std::cout << "Channel type is BoolType" << std::endl;
						break;
					case Bifrost::API::StringClassType : //,	/*!< Type Bifrost::API::String class */
						std::cout << "Channel type is StringClassType" << std::endl;
						break;
					case Bifrost::API::DictionaryClassType : //,	/*!< Type Bifrost::API::Dictionary class */
						std::cout << "Channel type is DictionaryClassType" << std::endl;
						break;
					case Bifrost::API::UInt64V2Type : //,	/*!< Type amino::Math::vec2ui64 */
						std::cout << "Channel type is UInt64V2Type" << std::endl;
						break;
					case Bifrost::API::UInt64V3Type : //,	/*!< Type amino::Math::vec3ui64 */
						std::cout << "Channel type is UInt64V3Type" << std::endl;
						break;
					case Bifrost::API::UInt64V4Type : //,	/*!< Type amino::Math::vec4ui64 */
						std::cout << "Channel type is UInt64V4Type" << std::endl;
						break;
					case Bifrost::API::StringArrayClassType : // /*!< Type Bifrost::API::StringArray class */
						std::cout << "Channel type is StringArrayClassType" << std::endl;
						break;
					case Bifrost::API::NoneType : // = 0,		/*!< Undefined data type */
						std::cout << "Channel type is NoneType" << std::endl;
						break;
					}
				}
			}
		}
	}


#ifdef NICHOLAS
	int positionChannelIndex = findChannelIndexViaName(component, position_channel_name.c_str());
	if (positionChannelIndex<0)
		return 1;
	const Bifrost::API::Channel& position_ch = component.channels()[positionChannelIndex];
	if (!position_ch.valid())
		return 1;
	if (position_ch.dataType() != Bifrost::API::FloatV3Type)
		return 1;

	Bifrost::API::Layout layout = component.layout();
	size_t depthCount = layout.depthCount();
	for (size_t d = 0; d<depthCount; d++) {
		for (size_t t = 0; t<layout.tileCount(d); t++) {
			Bifrost::API::TreeIndex tindex(t, d);
			if (!position_ch.elementCount(tindex)) {
				// nothing there
				continue;
			}

			const Bifrost::API::TileData<amino::Math::vec3f>& position_tile_data = position_ch.tileData<amino::Math::vec3f>(tindex);
			// const Bifrost::API::TileData<amino::Math::vec3f>& velocity_tile_data = velocity_ch.tileData<amino::Math::vec3f>( tindex );
			for (size_t i = 0; i<position_tile_data.count(); i++) {
				bounds.extendBy(Imath::V3f(position_tile_data[i][0],
					position_tile_data[i][1],
					position_tile_data[i][2]));
			}
		}
	}


#endif // NICHOLAS
	std::cout << "process_VoxelComponentType() END" << std::endl;
}

int process_bifrost_voxel(const std::string& bifrost_filename)
{
	// using namespace Bifrost::API;

	Bifrost::API::String biffile = bifrost_filename.c_str();
	Bifrost::API::ObjectModel om;
	Bifrost::API::FileIO fileio = om.createFileIO(biffile);
	Bifrost::API::StateServer ss = fileio.load();
	if (ss.valid())
	{
		const Bifrost::API::BIF::FileInfo& info = fileio.info();

		std::cout << boost::format("Version        : %1%") % info.version << std::endl;
		std::cout << boost::format("Frame          : %1%") % info.frame << std::endl;
		std::cout << boost::format("Channel count  : %1%") % info.channelCount << std::endl;
		std::cout << boost::format("Component name : %1%") % info.componentName.c_str() << std::endl;
		std::cout << boost::format("Component type : %1%") % info.componentType.c_str() << std::endl;
		std::cout << boost::format("Object name    : %1%") % info.objectName.c_str() << std::endl;
		std::cout << boost::format("Layout name    : %1%") % info.layoutName.c_str() << std::endl;

		StringContainer channel_names;
		for (size_t channelIndex = 0; channelIndex<info.channelCount; channelIndex++)
		{
			std::cout << std::endl;
			const Bifrost::API::BIF::FileInfo::ChannelInfo& channelInfo = fileio.channelInfo(channelIndex);
			std::cout << boost::format("\t""Channel name  : %1%") % channelInfo.name.c_str() << std::endl;
			std::cout << boost::format("\t""Data type     : %1%") % channelInfo.dataType << std::endl;
			std::cout << boost::format("\t""Max depth     : %1%") % channelInfo.maxDepth << std::endl;
			std::cout << boost::format("\t""Tile count    : %1%") % channelInfo.tileCount << std::endl;
			std::cout << boost::format("\t""Element count : %1%") % channelInfo.elementCount << std::endl;
			channel_names.push_back(channelInfo.name);
		}

		size_t numComponents = ss.components().count();
		std::cout << boost::format("StateServer components count : %1%") % numComponents << std::endl;
		for (size_t componentIndex = 0; componentIndex<numComponents; componentIndex++)
		{
			Bifrost::API::Component component = ss.components()[componentIndex];
			Bifrost::API::TypeID componentType = component.type();
			std::cout << boost::format("component[%1%] of type %2%") % componentIndex % componentType << std::endl;
			if (componentType == Bifrost::API::VoxelComponentType)
			{
				process_VoxelComponentType(component);
			}
		}


	}

	return 0;
}

int process_bifrost_voxel_autodesk(const std::string& bifrost_filename)
{
	using namespace Bifrost::API;

	String biffile = bifrost_filename.c_str();
	ObjectModel om;
	FileIO fileio = om.createFileIO(biffile);
	StateServer ss = fileio.load();
	std::cout << "00000" << std::endl;
	if (ss) {
		std::cout << "01000" << std::endl;
		// traverse the tree and iterate tile children
		RefArray layouts = ss.layouts();
		Layout layout = layouts[0];
		TileAccessor tacc = layout.tileAccessor();
		size_t depth = layout.maxDepth();
		TileIterator it = layout.tileIterator(depth, depth, BreadthFirst);

		while (it) {
			std::cout << "02000" << std::endl;
			// Get tile info to access children
			Tile tile = *it;
			TreeIndex itIndex = it.index();
			TreeIndex tindex = tile.index();
			TileInfo info = tile.info();
		
			// access children with i,j,k coordinates
			if (info.hasChildren) {
				std::cout << "03000" << std::endl;
				TileDimInfo dinfo = info.dimInfo;
				for (int i = 0; i<dinfo.tileWidth; i++) {
					for (int j = 0; j<dinfo.tileWidth; j++) {
						for (int k = 0; k<dinfo.tileWidth; k++) {
							std::cout << "04000" << std::endl;
							TreeIndex cindex = tile.child(i, j, k);

							// log the child tile info
							if (cindex.valid()) {
								Tile child = tacc.tile(cindex);
								TileInfo cinfo = child.info();

								std::cout << "tile id: " << cinfo.id << std::endl;
								std::cout << "	tile is valid: " << cinfo.valid << std::endl;
								std::cout << "	tile space coordinates: " << cinfo.i << ", " << cinfo.j << ", " << cinfo.k << std::endl;
								std::cout << "	tile index: " << cinfo.tile << ":" << cinfo.depth << std::endl;
								std::cout << "	tile parent: " << cinfo.parent << std::endl;
								std::cout << "	tile has children: " << cinfo.hasChildren << std::endl;
							}
						}
					}
				}
			}
			++it;
		}
	}
	return 0;
}

int process_bifrost_file(const std::string& bifrost_filename,
                         const std::string& position_channel_name,
                         const std::string& velocity_channel_name,
                         BBOX bbox_type,
                         const float* fps=0)
{
    Bifrost::API::String biffile = bifrost_filename.c_str();
    Bifrost::API::ObjectModel om;
    Bifrost::API::FileIO fileio = om.createFileIO( biffile );
    const Bifrost::API::BIF::FileInfo& info = fileio.info();

    std::cout << boost::format("Version        : %1%") % info.version << std::endl;
    std::cout << boost::format("Frame          : %1%") % info.frame << std::endl;
    std::cout << boost::format("Channel count  : %1%") % info.channelCount << std::endl;
    std::cout << boost::format("Component name : %1%") % info.componentName.c_str() << std::endl;
    std::cout << boost::format("Component type : %1%") % info.componentType.c_str() << std::endl;
    std::cout << boost::format("Object name    : %1%") % info.objectName.c_str() << std::endl;
    std::cout << boost::format("Layout name    : %1%") % info.layoutName.c_str() << std::endl;

    for (size_t channelIndex=0;channelIndex<info.channelCount;channelIndex++)
    {
        std::cout << std::endl;
        const Bifrost::API::BIF::FileInfo::ChannelInfo& channelInfo = fileio.channelInfo(channelIndex);
        std::cout << boost::format("        Channel name  : %1%") % channelInfo.name.c_str() << std::endl;
        std::cout << boost::format("        Data type     : %1%") % channelInfo.dataType << std::endl;
        std::cout << boost::format("        Max depth     : %1%") % channelInfo.maxDepth << std::endl;
        std::cout << boost::format("        Tile count    : %1%") % channelInfo.tileCount << std::endl;
        std::cout << boost::format("        Element count : %1%") % channelInfo.elementCount << std::endl;
    }

    if (bbox_type != BBOX::None)
    {
        // Need to load the entire file's content to process
        Bifrost::API::StateServer ss = fileio.load( );
        if (ss.valid())
        {
            size_t numComponents = ss.components().count();
            std::cout << boost::format("StateServer components count : %1%") % numComponents << std::endl;
            for (size_t componentIndex=0;componentIndex<numComponents;componentIndex++)
            {
                Bifrost::API::Component component = ss.components()[componentIndex];
                Bifrost::API::TypeID componentType = component.type();
                if (componentType == Bifrost::API::PointComponentType)
                    PointComponentTypeBBox(bbox_type,
                                           position_channel_name,
                                           velocity_channel_name,
                                           component,
                                           fps);
                else if (componentType == Bifrost::API::VoxelComponentType)
                	VoxelComponentTypeBBox(bbox_type, component);
            }
        }
        else
        {
            std::cerr << boost::format("Unable to load the content of the Bifrost file \"%1%\"") % bifrost_filename.c_str()
                      << std::endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{

	try {
		typedef std::vector<std::string> StringContainer;
		std::string position_channel_name("position");
		std::string velocity_channel_name("velocity");
		BBOX bbox_type = BBOX::None;
		std::string bifrost_filename;
		float fps = 24.0f;
		po::options_description desc("Allowed options");
		desc.add_options()
			("version", "print version string")
			("help", "produce help message")
			("bbox", po::value<BBOX>(&bbox_type), "Analyze the entire file to obtain the overall bounding box [0:None, 1:PointsOnly, 2:PointsWithVelocity]")
			("fps", po::value<float>(&fps),
				"Frames per second to scale velocity when determining the velocity-attenuated bounding box. Defaults to 24.0")
				("input-file", po::value<std::vector<std::string> >(),
					"input files")
			;

		po::positional_options_description p;
		p.add("input-file", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc << "\n";
			return 1;
		}
		if (vm.count("input-file"))
		{
			std::cout << "Input files are: "
				<< vm["input-file"].as< std::vector<std::string> >() << "\n";
			if (vm["input-file"].as< std::vector<std::string> >().size() == 1)
			{
				bifrost_filename = vm["input-file"].as< std::vector<std::string> >()[0];
			}
			else
			{
				std::cout << desc << "\n";
				return 1;
			}
		}
		std::cout << "fps = " << fps << std::endl;
		if (bifrost_filename.size() > 0)
		{
			process_bifrost_voxel(bifrost_filename);
			/*
			process_bifrost_file(bifrost_filename,
				position_channel_name,
				velocity_channel_name,
				bbox_type,
				bbox_type == BBOX::PointsWithVelocity ? &fps : 0);
				*/
		}
		else
        {
            std::cout << desc << "\n";
            return 1;
        }
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!\n";
    }

    return 0;

}
