#pragma once

#include <BifrostHeaders.h>

int findChannelIndexViaName(const Bifrost::API::Component& component,
                            const Bifrost::API::String& searchChannelName);

void
get_channel(const Bifrost::API::Component& i_component,
			const std::string& i_channel_name,
			const Bifrost::API::DataType& i_expected_type,
			Bifrost::API::Channel& channel,
			bool& o_status);
