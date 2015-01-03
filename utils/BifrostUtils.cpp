#include "BifrostUtils.h"

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
