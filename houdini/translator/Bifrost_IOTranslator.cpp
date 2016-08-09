#include "Bifrost_IOTranslator.h"
#include <string.h>
#include <boost/format.hpp>

typedef std::vector<amino::Math::vec3f> V3fContainer;
typedef std::vector<float> FloatContainer;
typedef std::vector<int> IntegerContainer;

Bifrost_IOTranslator::BifrostChannelNameToHoudiniAttributeNameMap Bifrost_IOTranslator::initializeChannelAttributeMap()
{
	BifrostChannelNameToHoudiniAttributeNameMap caMap;
	caMap.insert(std::pair<std::string,std::string>("density","density"));
	caMap.insert(std::pair<std::string,std::string>("droplet","droplet"));
	caMap.insert(std::pair<std::string,std::string>("expansionRate","expansionRate"));
	caMap.insert(std::pair<std::string,std::string>("id64","id"));
	caMap.insert(std::pair<std::string,std::string>("position","P"));
	caMap.insert(std::pair<std::string,std::string>("stictionBandwidth","stictionBandwidth"));
	caMap.insert(std::pair<std::string,std::string>("stictionStrength","stictionStrength"));
	caMap.insert(std::pair<std::string,std::string>("uv","uv"));
	caMap.insert(std::pair<std::string,std::string>("velocity","v"));
	caMap.insert(std::pair<std::string,std::string>("vorticity","vorticity"));

	return caMap;
}

Bifrost_IOTranslator::BifrostChannelNameToHoudiniAttributeNameMap Bifrost_IOTranslator::_bcn2han_map = Bifrost_IOTranslator::initializeChannelAttributeMap();

Bifrost_IOTranslator::Bifrost_IOTranslator()
{
}

Bifrost_IOTranslator::Bifrost_IOTranslator(const Bifrost_IOTranslator &src)
{
}

Bifrost_IOTranslator::~Bifrost_IOTranslator()
{
}

GEO_IOTranslator *
Bifrost_IOTranslator::duplicate() const
{
    return new Bifrost_IOTranslator(*this);
}

const char *
Bifrost_IOTranslator::formatName() const
{
    return (boost::format("Autodesk Maya Bifrost Format v2 (Read-Only) built : %1% %2%\n\tExtensions: .bif") % __DATE__ % __TIME__).str().c_str();
}

int
Bifrost_IOTranslator::checkExtension(const char *name)
{
    UT_String		sname(name);

    if (sname.fileExtension() && !strcmp(sname.fileExtension(), ".bif"))
    {
    	return true;
    }
    return false;
}

int
Bifrost_IOTranslator::checkMagicNumber(unsigned magic)
{
    return 0;
}

template<typename T>
bool Bifrost_IOTranslator::processChannelData(Bifrost::API::Component& component,
												 Bifrost::API::Channel& channel_data,
												 bool i_is_point_position,
												 std::vector<T>& o_channel_data_array) const
{
	// if ( channel_data ) {
		Bifrost::API::Layout layout = component.layout();
		float voxel_scale = layout.voxelScale();
		// std::cout << boost::format("Voxel Scale = %1%") % voxel_scale << std::endl;
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
	//}
	return true;
}

GA_Detail::IOStatus
Bifrost_IOTranslator::fileLoad(GEO_Detail *gdp, UT_IStream &is, bool ate_magic)
{
    // Bifrost file handling
    Bifrost::API::String biffile = is.getFilename();

	Bifrost::API::ObjectModel om;
	Bifrost::API::FileIO fileio = om.createFileIO( biffile );
	Bifrost::API::StateServer ss = fileio.load( );
    const Bifrost::API::BIF::FileInfo& info = fileio.info();

	if ( !ss.valid() ) {
        std::cerr << boost::format("Unable to load the content of the Bifrost file \"%1%\"") % is.getFilename()
                  << std::endl;
        return GA_Detail::IOStatus(false);
	}

	Bifrost::API::Component component = ss.components()[0];
	if ( component.type() != Bifrost::API::PointComponentType ) {
		std::cerr << "Wrong component (" << component.type() << ")" << std::endl;
	    return GA_Detail::IOStatus(false);
	}

	// float bifrost_scale = 0.0;

	Bifrost::API::RefArray channels = component.channels();
    for (size_t channelIndex=0;channelIndex<info.channelCount;channelIndex++)
    {
        const Bifrost::API::BIF::FileInfo::ChannelInfo& channelInfo = fileio.channelInfo(channelIndex);

        BifrostChannelNameToHoudiniAttributeNameMap::const_iterator nameMappingIter = _bcn2han_map.begin();
        BifrostChannelNameToHoudiniAttributeNameMap::const_iterator nameMappingEIter = _bcn2han_map.end();
        for (;nameMappingIter!=nameMappingEIter;++nameMappingIter)
        {
        	if (channelInfo.name.find(nameMappingIter->first.c_str()) != Bifrost::API::String::npos)
        	{
				 bool is_point_position = channelInfo.name.find("position") != Bifrost::API::String::npos;

        		switch (channelInfo.dataType)
        		{
        		case		Bifrost::API::FloatType:		/*!< Defines a channel of type float. #1 */
					{
						typedef float DataType;
						std::vector<DataType> channel_data_array;
						Bifrost::API::Channel channel = channels[channelIndex];

						bool successfully_processed = processChannelData<DataType>(component,channel,is_point_position,channel_data_array);
						if (successfully_processed)
						{
							size_t numParticles = channel_data_array.size();

							GA_RWHandleF float_attrib(gdp->findAttribute(GA_ATTRIB_POINT,nameMappingIter->second.c_str()));
							if (!float_attrib.isValid())
							{
							    float_attrib.bind(gdp->addFloatTuple(GA_ATTRIB_POINT, nameMappingIter->second.c_str(), 1));
							}

							float_attrib.getAttribute()->setTypeInfo(GA_TYPE_VOID);

					        UT_ValArray<GA_RWHandleF::BASETYPE> float_array(numParticles);
					        for (size_t i = 0; i<numParticles;i++)
						        float_array.array()[i] = channel_data_array[i];
					    	gdp->setAttributeFromArray(float_attrib.getAttribute(),gdp->getPointRange(),float_array);


						}
					}
        			break;
        		case		Bifrost::API::FloatV2Type:	/*!< Defines a channel of type amino::Math::vec2f. #2 */
					{
						typedef amino::Math::vec2f DataType;
						std::vector<DataType> channel_data_array;
						Bifrost::API::Channel channel = channels[channelIndex];

						bool successfully_processed = processChannelData<DataType>(component,channel,is_point_position,channel_data_array);
						if (successfully_processed)
						{
							size_t numParticles = channel_data_array.size();

							GA_RWHandleV2 v2_attrib(gdp->findAttribute(GA_ATTRIB_POINT,nameMappingIter->second.c_str()));
							if (!v2_attrib.isValid())
							{
								v2_attrib.bind(gdp->addFloatTuple(GA_ATTRIB_POINT, nameMappingIter->second.c_str(), 2));
							}

							v2_attrib.getAttribute()->setTypeInfo(GA_TYPE_VOID);

							UT_ValArray<UT_Vector2> v2_array(numParticles);
							for (size_t i = 0; i<numParticles;i++)
								v2_array.array()[i].assign(channel_data_array[i].v[0],channel_data_array[i].v[1]);
							gdp->setAttributeFromArray(v2_attrib.getAttribute(),gdp->getPointRange(),v2_array);
						}
					}
        			break;
        		case		Bifrost::API::FloatV3Type:	/*!< Defines a channel of type amino::Math::vec3f. #3 */
					{
						typedef amino::Math::vec3f DataType;
						std::vector<DataType> channel_data_array;
						Bifrost::API::Channel channel = channels[channelIndex];

						bool successfully_processed = processChannelData<DataType>(component,channel,is_point_position,channel_data_array);
						if (successfully_processed)
						{
							size_t numParticles = channel_data_array.size();
							if (is_point_position)
							{
								/*GA_Offset p_offset = */gdp->appendPointBlock(numParticles);
								GA_Range p_range = gdp->getPointRange();

								UT_ValArray<UT_Vector3> v3_array(numParticles);
								for (size_t i = 0; i<numParticles;i++)
									v3_array.array()[i].assign(channel_data_array[i].v[0],channel_data_array[i].v[1],channel_data_array[i].v[2]);
								gdp->setPos3FromArray(p_range,v3_array);
							}
							else
							{
								GA_RWHandleV3 v3_attrib(gdp->findAttribute(GA_ATTRIB_POINT,nameMappingIter->second.c_str()));
								if (!v3_attrib.isValid())
								{
								    v3_attrib.bind(gdp->addFloatTuple(GA_ATTRIB_POINT, nameMappingIter->second.c_str(), 3));
								}

								v3_attrib.getAttribute()->setTypeInfo(GA_TYPE_VECTOR);

						        UT_ValArray<UT_Vector3> v3_array(numParticles);
						        for (size_t i = 0; i<numParticles;i++)
							        v3_array.array()[i].assign(channel_data_array[i].v[0],channel_data_array[i].v[1],channel_data_array[i].v[2]);
						    	gdp->setAttributeFromArray(v3_attrib.getAttribute(),gdp->getPointRange(),v3_array);

							}
						}
					}
        			break;
        		case		Bifrost::API::Int32Type:		/*!< Defines a channel of type int32_t. #4 */
        			break;
        		case		Bifrost::API::Int64Type:		/*!< Defines a channel of type int64_t. #5 */
        			break;
        		case		Bifrost::API::UInt32Type:		/*!< Defines a channel of type uint32_t. #6 */
        			break;
        		case		Bifrost::API::UInt64Type:		/*!< Defines a channel of type uint64_t. #7 */
					{
						/*!
						 * \remark Houdini does not have (at this moment) have an 64bit unsigned integer,
						 *         we have to use a 64bit signed integer instead
						 */
						typedef uint64_t DataType;
						std::vector<DataType> channel_data_array;
						Bifrost::API::Channel channel = channels[channelIndex];

						bool successfully_processed = processChannelData<DataType>(component,channel,is_point_position,channel_data_array);
						if (successfully_processed)
						{
							size_t numParticles = channel_data_array.size();

							GA_RWHandleID uint64_attrib(gdp->findAttribute(GA_ATTRIB_POINT,nameMappingIter->second.c_str()));
							if (!uint64_attrib.isValid())
							{
								uint64_attrib.bind(gdp->addTuple(GA_STORE_INT64, GA_ATTRIB_POINT, nameMappingIter->second.c_str(), 1));
							}

							uint64_attrib.getAttribute()->setTypeInfo(GA_TYPE_NONARITHMETIC_INTEGER);

							UT_ValArray<GA_RWHandleID::BASETYPE> int64_array(numParticles);
							for (size_t i = 0; i<numParticles;i++)
								int64_array.array()[i] = channel_data_array[i];
							gdp->setAttributeFromArray(uint64_attrib.getAttribute(),gdp->getPointRange(),int64_array);

						}
					}
        			break;
        		case		Bifrost::API::Int32V2Type:	/*!< Defines a channel of type amino::Math::vec2i. #8 */
        			break;
        		case		Bifrost::API::Int32V3Type:		/*!< Defines a channel of type amino::Math::vec3i. #9 */
        			break;
				default:
					break;
        		}
        	}
        }
    }

    // All done successfully
    return GA_Detail::IOStatus(true);
}

GA_Detail::IOStatus
Bifrost_IOTranslator::fileSave(const GEO_Detail *gdp, std::ostream &os)
{
    return GA_Detail::IOStatus(false);
}

void
newGeometryIO(void *)
{
    GU_Detail::registerIOTranslator(new Bifrost_IOTranslator());

    // Note due to the just-in-time loading of GeometryIO, the f3d
    // won't be added until after your first f3d save/load.
    // Thus this is replicated in the newDriverOperator.
    UT_ExtensionList		*geoextension;
    geoextension = UTgetGeoExtensions();
    if (!geoextension->findExtension("bif"))
	geoextension->addExtension("bif");
}
