#pragma once

// Houdini header - START
#include <GU/GU_Detail.h>
#include <GU/GU_PrimVolume.h>
#include <GEO/GEO_AttributeHandle.h>
#include <GEO/GEO_IOTranslator.h>
#include <SOP/SOP_Node.h>
#include <UT/UT_Assert.h>
#include <UT/UT_IOTable.h>
// Houdini header - END

// Bifrost headers - START
#include <bifrostapi/bifrost_om.h>
#include <bifrostapi/bifrost_stateserver.h>
#include <bifrostapi/bifrost_component.h>
#include <bifrostapi/bifrost_fileio.h>
#include <bifrostapi/bifrost_fileutils.h>
#include <bifrostapi/bifrost_string.h>
#include <bifrostapi/bifrost_channel.h>
#include <bifrostapi/bifrost_tiledata.h>
#include <bifrostapi/bifrost_types.h>
#include <bifrostapi/bifrost_layout.h>
// Bifrost headers - END

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>

class Bifrost_IOTranslator : public GEO_IOTranslator
{
	typedef std::map<std::string,std::string> BifrostChannelNameToHoudiniAttributeNameMap;
	static BifrostChannelNameToHoudiniAttributeNameMap _bcn2han_map;
	static BifrostChannelNameToHoudiniAttributeNameMap initializeChannelAttributeMap();
//	enum DataType {
//		NoneType=0,		/*!< Undefined data type. Uninitialized %Channel object are set to %NoneType. */
//		FloatType,		/*!< Defines a channel of type float. */
//		FloatV2Type,	/*!< Defines a channel of type amino::Math::vec2f. */
//		FloatV3Type,	/*!< Defines a channel of type amino::Math::vec3f. */
//		Int32Type,		/*!< Defines a channel of type int32_t. */
//		Int64Type,		/*!< Defines a channel of type int64_t. */
//		UInt32Type,		/*!< Defines a channel of type uint32_t. */
//		UInt64Type,		/*!< Defines a channel of type uint64_t. */
//		Int32V2Type,	/*!< Defines a channel of type amino::Math::vec2i. */
//		Int32V3Type		/*!< Defines a channel of type amino::Math::vec3i. */
//	};

	template<typename T>
	bool processChannelData(Bifrost::API::Component& component,
							Bifrost::API::Channel& velocity_channel,
							bool i_is_point_position,
							std::vector<T>& o_channel_data_array) const;
public:
	Bifrost_IOTranslator();
	Bifrost_IOTranslator(const Bifrost_IOTranslator &src);
	virtual ~Bifrost_IOTranslator();

    virtual GEO_IOTranslator    *duplicate() const;
    virtual const char *formatName() const;
    virtual int         checkExtension(const char *name);
    virtual int         checkMagicNumber(unsigned magic);
    virtual GA_Detail::IOStatus  fileLoad(GEO_Detail *, UT_IStream &,
                                          bool ate_magic);
    /*!
     * \brief Saving to Bifrost is not support
     * \return Returning false
     */
    virtual GA_Detail::IOStatus  fileSave(const GEO_Detail *, std::ostream &);
};
