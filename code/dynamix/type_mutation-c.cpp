// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "../dnmx/type_mutation.h"
#include "type_mutation.hpp"
#include "domain.hpp"
#include "type.hpp"

using namespace dynamix;

extern "C" {

#define self type_mutation::from_c_handle(hmut)

dnmx_type_mutation_handle dnmx_create_type_mutation_empty(dnmx_domain_handle hd) {
    try {
        auto mut = new type_mutation(*domain::from_c_handle(hd));
        return mut->to_c_hanlde();
    }
    catch (std::exception&) {
        return nullptr;
    }
}

dnmx_type_mutation_handle dnmx_create_type_mutation_from_type(dnmx_type_handle ht) {
    try {
        auto mut = new type_mutation(*type::from_c_handle(ht));
        return mut->to_c_hanlde();
    }
    catch (std::exception&) {
        return nullptr;
    }
}

void dnmx_destroy_unused_type_mutation(dnmx_type_mutation_handle hmut) {
    delete self;
}

dnmx_domain_handle dnmx_type_mutation_get_domain(dnmx_type_mutation_handle hmut) {
    return self->dom.to_c_hanlde();
}

bool dnmx_type_mutation_has(dnmx_type_mutation_handle hmut, const dnmx_mixin_info* info) {
    return self->has(*info);
}
const dnmx_mixin_info* dnmx_type_mutation_has_by_name(dnmx_type_mutation_handle hmut, dnmx_sv name) {
    return self->has(name.to_std());
}
bool dnmx_type_mutation_implements_strong(dnmx_type_mutation_handle hmut, const dnmx_feature_info* info) {
    return self->implements_strong(*info);
}
const dnmx_feature_info* dnmx_type_mutation_implements_strong_by_name(dnmx_type_mutation_handle hmut, dnmx_sv name) {
    return self->implements_strong(name.to_std());
}
bool dnmx_type_mutation_implements(dnmx_type_mutation_handle hmut, const dnmx_feature_info* info) {
    return self->implements(*info);
}

bool dnmx_type_mutation_add(dnmx_type_mutation_handle hmut, const dnmx_mixin_info* info) {
    try {
        self->add(*info);
        return true;
    }
    catch (std::exception&) {
        return false;
    }
}
bool dnmx_type_mutation_add_if_lacking(dnmx_type_mutation_handle hmut, const dnmx_mixin_info* info) {
    try {
        return self->add_if_lacking(*info);
    }
    catch (std::exception&) {
        return false;
    }
}
bool dnmx_type_mutation_remove(dnmx_type_mutation_handle hmut, const dnmx_mixin_info* info) {
    return self->remove(*info);
}
const dnmx_mixin_info* dnmx_type_mutation_remove_by_name(dnmx_type_mutation_handle hmut, dnmx_sv name) {
    return self->remove(name.to_std());
}
bool dnmx_type_mutation_to_back(dnmx_type_mutation_handle hmut, const dnmx_mixin_info* info) {
    try {
        self->to_back(*info);
        return true;
    }
    catch (std::exception&) {
        return false;
    }
}
const dnmx_mixin_info* dnmx_type_mutation_to_back_by_name(dnmx_type_mutation_handle hmut, dnmx_sv name) {
    try {
        return &self->to_back(name.to_std());
    }
    catch (std::exception&) {
        return nullptr;
    }
}
void dnmx_type_mutation_dedup(dnmx_type_mutation_handle hmut) {
    self->dedup();
}

const dnmx_mixin_info* const* dnmx_type_mutation_get_mixins(dnmx_type_mutation_handle hmut, dnmx_mixin_index_t* out_num_mixins) {
    const auto& vec = self->mixins;
    *out_num_mixins = dnmx_mixin_index_t(vec.size());
    return vec.data();
}

bool dnmx_type_mutation_set_mixins(dnmx_type_mutation_handle hmut, const dnmx_mixin_info* const* mixins, dnmx_mixin_index_t num_mixins) {
    try {
        self->mixins.assign(mixins, mixins + num_mixins);
        return true;
    }
    catch (std::exception&) {
        return false;
    }
}

}