// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "declare_type_class.hpp"
#include "globals.hpp"
#include "domain.hpp"
#include "type.hpp"

#include <type_traits>

namespace dynamix::impl {
struct registered_type_class_instance {
    domain& dom;
    const type_class& tc;
    registered_type_class_instance(domain& dom, const type_class& tc) : dom(dom), tc(tc) {
        dom.register_type_class(tc);
    }
    ~registered_type_class_instance() {
        dom.unregister_type_class(tc);
    }
};
}

namespace dynamix::util {
// call a type class match function which can be bool(const dynamix::type&) or bool(dnmx_type_handle)
template <typename Func>
bool call_type_class_match_func(Func&& func, dnmx_type_handle ht) {
    if constexpr (std::is_invocable_r_v<bool, Func, const type&>) {
        return func(*type::from_c_handle(ht));
    }
    else {
        static_assert(std::is_invocable_r_v<bool, Func, dnmx_type_handle>,
            "type class match function must be bool(const dynamix::type&) or bool(dnmx_type_handle)");
        return func(ht);
    }
}

// convert a function to a type class match function (dnmx_type_class_match_func)
// Func can be:
// * bool(dnmx_type_handle) noexcept: used as is
// * bool(const dynamix::type&) or bool(dnmx_type_handle): wrapped in a captureless lambda
// the match function is noexcept, so if a wrapped function throws, std::terminate will be called
template <auto Func>
constexpr dnmx_type_class_match_func make_type_class_match_func() noexcept {
    if constexpr (std::is_convertible_v<decltype(Func), dnmx_type_class_match_func>) {
        return Func;
    }
    else {
        return [](dnmx_type_handle ht) noexcept -> bool { return call_type_class_match_func(Func, ht); };
    }
}
}

// define a type class with a match function
// func can be anything that can be called as bool(const dynamix::type&) or bool(dnmx_type_handle):
// a function name or a captureless lambda
// the match function is noexcept, so if it throws, std::terminate will be called
#define DYNAMIX_DEFINE_TYPE_CLASS_WITH(domain_tag, tc, func) \
    const dynamix::type_class tc::m_dynamix_type_class = {dnmx_make_sv_lit(#tc), \
        [](dnmx_type_handle _dynamix_ht) noexcept -> bool { return ::dynamix::util::call_type_class_match_func(func, _dynamix_ht); }}; \
    static ::dynamix::impl::registered_type_class_instance I_DNMX_PP_CAT(_dynamix_type_class_, tc)(::dynamix::g::get_domain<domain_tag>(), tc::m_dynamix_type_class)
