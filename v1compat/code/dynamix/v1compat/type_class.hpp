// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "domain.hpp"
#include <dynamix/type.hpp>
#include <dynamix/type_class.hpp>
#include <dynamix/declare_type_class.hpp>
#include <dynamix/define_type_class.hpp>

namespace dynamix::v1compat {
class type_class : public dynamix::type_class {
public:
    using match_func = dnmx_type_class_match_func;
    explicit type_class(match_func mf, bool /*ignored*/ = false) {
        name = {};
        matches = mf;
    }
};

}

#define DYNAMIX_V1_TYPE_CLASS(tc) DYNAMIX_DECLARE_TYPE_CLASS(tc)

#define DYNAMIX_V1_DEFINE_TYPE_CLASS(tc) \
    static bool I_DNMX_PP_CAT(_implement_, tc)(const ::dynamix::type&); \
    DYNAMIX_DEFINE_TYPE_CLASS_WITH(::dynamix::v1compat::domain_tag, tc, [](dnmx_type_handle th) noexcept { \
        return I_DNMX_PP_CAT(_implement_, tc)(*::dynamix::type::from_c_handle(th)); \
    }); \
    bool I_DNMX_PP_CAT(_implement_, tc)(const ::dynamix::type& type)
