#pragma once

// Bifrost headers - START
#include <bifrostapi/bifrost_om.h>
#include <bifrostapi/bifrost_stateserver.h>
#include <bifrostapi/bifrost_component.h>
#include <bifrostapi/bifrost_fileio.h>
#include <bifrostapi/bifrost_fileutils.h>
#include <bifrostapi/bifrost_string.h>
// #include <bifrostapi/bifrost_stringarray.h>
#include <bifrostapi/bifrost_refarray.h>
#include <bifrostapi/bifrost_channel.h>
#include <bifrostapi/bifrost_layout.h>
// Bifrost headers - END

int findChannelIndexViaName(const Bifrost::API::Component& component,
                            const Bifrost::API::String& searchChannelName);
