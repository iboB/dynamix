// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "mixin_info_fwd.hpp"
#include "type_class.hpp"
#include <iosfwd>

enum dnmx_dbg_dmp_flags {
    dnmx_dom_dmp_features = 0x1,
    dnmx_dom_dmp_mixins = 0x2,
    dnmx_dom_dmp_mutation_rules = 0x4,
    dnmx_dom_dmp_type_classes = 0x8,
    dnmx_dom_dmp_types = 0x10,
    dnmx_dom_dmp_type_queries = 0x20,
    dnmx_dom_dmp_settings = 0x40,
    dnmx_dom_dmp_all = 0xFF,

    dnmx_mixin_dmp_ex = 0x100,
    dnmx_mixin_dmp_features = 0x200,
    dmnx_mixin_dmp_all = 0xF00,

    dnmx_type_dmp_ex = 0x1000,
    dnmx_type_dmp_ftable = 0x2000,
    dnmx_type_dmp_full_ftable = 0x4000,
    dnmx_type_dmp_matching_type_classes = 0x8000,
    dnmx_type_dmp_all = 0xF000,

    dnmx_tc_dmp_matching_types = 0x10000,
    dnmx_tc_dmp_all = 0xF0000,

    dnmx_dmp_all = 0xFFFFFFFF
};

namespace dynamix {
class domain;
class type;
class object;

DYNAMIX_API void dbg_dmp(std::ostream& out, const domain& d, int flags = dnmx_dmp_all);
DYNAMIX_API void dbg_dmp(std::ostream& out, const mixin_info& mi, int flags = dmnx_mixin_dmp_all);
DYNAMIX_API void dbg_dmp(std::ostream& out, const type& t, int flags = dnmx_type_dmp_all);
DYNAMIX_API void dbg_dmp(std::ostream& out, const type_class& tc, int flags = dnmx_tc_dmp_all);

DYNAMIX_API void dbg_dmp(std::ostream& out, const object& o);
}
