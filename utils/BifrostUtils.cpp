#include "BifrostUtils.h"
#include <boost/format.hpp>

int findChannelIndexViaName(const Bifrost::API::Component& component,
                            const Bifrost::API::String& searchChannelName)
{
    Bifrost::API::RefArray channels = component.channels();
    size_t particleCount = component.elementCount();
    size_t channelCount = channels.count();
    for (size_t channelIndex=0;channelIndex<channelCount;channelIndex++)
    {
        const Bifrost::API::Channel& ch = channels[channelIndex];
        Bifrost::API::String channelName = ch.name();
        if (channelName.find(searchChannelName) != Bifrost::API::String::npos)
        {
            return channelIndex;
        }
    }
    return -1;
}

void
get_channel(const Bifrost::API::Component& i_component,
			const std::string& i_channel_name,
			const Bifrost::API::DataType& i_expected_type,
			Bifrost::API::Channel& o_channel,
			bool& o_status)
{
    int channelIndex = findChannelIndexViaName(i_component,i_channel_name.c_str());
    if (channelIndex<0)
    {
        std::cerr << boost::format("get_channel() : channel '%1%' not found") % i_channel_name << std::endl;
        o_status = false;
    }
    o_channel = i_component.channels()[channelIndex];
    if (!o_channel.valid())
    {
        std::cerr << boost::format("get_channel() : channel '%1%' not valid") % i_channel_name << std::endl;
        o_status = false;
    }
    if ( o_channel.dataType() != i_expected_type)
    {
        std::cerr << boost::format("get_channel() : channel '%1%' of type %2% is different from expected type %3%") % i_channel_name % o_channel.dataType() % i_expected_type << std::endl;
        o_status = false;
    }
    o_status = true;
}
