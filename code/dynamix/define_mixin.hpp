// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "domain.hpp"
#include "mixin_info_data.hpp"

#include "mixin_info_util.hpp"
#include "globals.hpp"

#include "../dnmx/bits/pp.h"

#include <vector>

namespace dynamix::impl {

template <typename DomainTag, typename Mixin>
struct mixin_info_data_instance {
    static util::mixin_info_data& the_data_safe() noexcept {
        static util::mixin_info_data data;
        return data;
    }

    mixin_info_data_instance(util::mixin_info_data_builder<Mixin>) {
        the_data_safe().register_in(::dynamix::g::get_domain<DomainTag>());
    }
    ~mixin_info_data_instance() {
        auto& data = the_data_safe();
        if (auto dom = data.info.dom) {
            data.unregister_from(*domain::from_c_handle(dom));
        }
    }
};
}

#define I_DYNAMIX_MIXIN_INFO_VAR(mixin) I_DNMX_PP_CAT(_dynamix_mixin_info_data_, mixin)

#define DYNAMIX_DEFINE_MIXIN(domain_tag, mixin) \
static const ::dynamix::util::mixin_info_data& I_DYNAMIX_MIXIN_INFO_VAR(mixin) = ::dynamix::impl::mixin_info_data_instance<domain_tag, mixin>::the_data_safe(); \
const ::dynamix::mixin_info& _dynamix_get_mixin_info(mixin*) { \
    return I_DYNAMIX_MIXIN_INFO_VAR(mixin).info; \
} \
using namespace ::dynamix::util::builder_literals; \
static ::dynamix::impl::mixin_info_data_instance<domain_tag, mixin> I_DNMX_PP_CAT(_dynamix_mixin_info_data_instance_for_, mixin) = \
    ::dynamix::util::mixin_info_data_builder<mixin>(::dynamix::impl::mixin_info_data_instance<domain_tag, mixin>::the_data_safe(), dnmx_make_sv_lit(I_DNMX_PP_STRINGIZE(mixin)))

