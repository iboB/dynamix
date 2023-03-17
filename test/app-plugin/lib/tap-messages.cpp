// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "tap-messages.hpp"
#include "tap-messages-traits.hpp"
#include "tap-object.hpp"
#include <dynamix/msg/define_msg.hpp>

namespace tap {
DYNAMIX_DEFINE_SIMPLE_MSG(simple_uni, unicast);

namespace {
std::string work_default(const object&, char c) {
    std::string ret = "working with ";
    ret += c;
    return ret;
}
}
DYNAMIX_DEFINE_MSG_EX(work_msg, unicast, true, work_default, work, std::string, (const object&, char));
DYNAMIX_DEFINE_MSG(set_target_msg, unicast, set_target, void, (object&, std::string_view));
DYNAMIX_DEFINE_MSG(dump_msg, multicast, dump, void, (const object&, std::vector<std::string>&));

}
