// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "tap-api.h"
#include <dynamix/msg/declare_msg.hpp>

#include <vector>
#include <string>
#include <string_view>

namespace tap {
class object;

DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(TAP_API, simple_uni, std::string&(object&, int));

DYNAMIX_DECLARE_EXPORTED_MSG(TAP_API, work_msg, work, std::string, (const object&, char));
DYNAMIX_DECLARE_EXPORTED_MSG(TAP_API, set_target_msg, set_target, void, (object&, std::string_view));
DYNAMIX_DECLARE_EXPORTED_MSG(TAP_API, dump_msg, dump, void, (const object&, std::vector<std::string>&));
}
