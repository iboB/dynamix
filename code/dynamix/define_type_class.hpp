// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "declare_type_class.hpp"
#include "globals.hpp"
#include "domain.hpp"
#include "type.hpp"

namespace dynamix::impl {
struct registered_type_class_instance {
    domain& dom;
    type_class& tc;
    registered_type_class_instance(domain& dom, type_class& tc) : dom(dom), tc(tc) {
        dom.register_type_class(tc);
    }
    ~registered_type_class_instance() {
        dom.unregister_type_class(tc);
    }
};
}

// define a type class with bool(const type&);
#define DYNAMIX_DEFINE_TYPE_CLASS_WITH(domain_tag, tc, func) \
    const dynamix::type_class tc::m_dynamix_type_class = {dnmx_make_sv_lit(#tc), func}; \
    static ::dynamix::impl::registered_type_class_instance I_DNMX_PP_CAT(_dynamix_type_class_, tc)(::dynamix::g::get_domain<domain_tag>(), tc::m_dynamix_type_class)
