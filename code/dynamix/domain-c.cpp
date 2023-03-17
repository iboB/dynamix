// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "../dnmx/domain.h"
#include "domain.hpp"
#include "type.hpp"

using namespace dynamix;

extern "C" {

#define self domain::from_c_handle(hd)

dnmx_domain_handle dnmx_create_domain(dnmx_sv name, dnmx_domain_settings settings, uintptr_t user_data, void* context) {
    auto new_dom = new dynamix::domain(name.to_std(), settings, user_data, context);
    return new_dom->to_c_hanlde();
}
void dnmx_destroy_domain(dnmx_domain_handle hd) {
    delete self;
}

void dnmx_set_domain_user_data(dnmx_domain_handle hd, uintptr_t user_data) {
    self->user_data = user_data;
}
void dnmx_set_domain_context(dnmx_domain_handle hd, void* context) {
    self->context = context;
}

dnmx_error_return_t dnmx_register_feature(dnmx_domain_handle hd, dnmx_feature_info* info) {
    try {
        self->register_feature(*info);
        return dnmx_result_success;
    }
    catch (std::exception&) {
        return -1;
    }
}
void dnmx_unregister_feature(dnmx_domain_handle hd, dnmx_feature_info* info) {
    self->unregister_feature(*info);
}

dnmx_error_return_t dnmx_register_mixin(dnmx_domain_handle hd, dnmx_mixin_info* info) {
    try {
        self->register_mixin(*info);
        return dnmx_result_success;
    }
    catch (std::exception&) {
        return -1;
    }
}
void dnmx_unregister_mixin(dnmx_domain_handle hd, dnmx_mixin_info* info) {
    self->unregister_mixin(*info);
}

const dnmx_feature_info* dnmx_get_feature_info_by_id(dnmx_domain_handle hd, dnmx_feature_id id) {
    return self->get_feature_info(id);
}

const dnmx_feature_info* dnmx_get_feature_info_by_name(dnmx_domain_handle hd, dnmx_sv name) {
    return self->get_feature_info(name.to_std());
}

const dnmx_mixin_info* dnmx_get_mixin_info_by_id(dnmx_domain_handle hd, dnmx_mixin_id id) {
    return self->get_mixin_info(id);
}

const dnmx_mixin_info* dnmx_get_mixin_info_by_name(dnmx_domain_handle hd, dnmx_sv name) {
    return self->get_mixin_info(name.to_std());
}

dnmx_type_handle dnmx_get_type(dnmx_domain_handle hd, const dnmx_mixin_info* const* mixins, uint32_t num_mixins) {
    try {
        return &self->get_type({mixins, num_mixins});
    }
    catch (std::exception&) {
        return nullptr;
    }
}

dnmx_type_handle dnmx_get_empty_type(dnmx_domain_handle hd) {
    return &self->get_empty_type();
}

size_t dnmx_get_num_types(dnmx_domain_handle hd) {
    return self->num_types();
}

void dnmx_garbage_collect_types(dnmx_domain_handle hd) {
    self->garbage_collect_types();
}

}
