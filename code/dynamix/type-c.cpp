// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "../dnmx/type.h"
#include "type.hpp"

using namespace dynamix;

extern "C" {

#define self type::from_c_handle(ht)

size_t dnmx_type_num_objects(dnmx_type_handle ht) {
    return self->num_objects();
}

dnmx_mixin_index_t dnmx_type_num_mixins(dnmx_type_handle ht) {
    return self->num_mixins();
}

bool dnmx_type_has(dnmx_type_handle ht, const dnmx_mixin_info* info) {
    return self->has(*info);
}
bool dnmx_type_has_by_name(dnmx_type_handle ht, dnmx_sv name) {
    return self->has(name.to_std());
}

bool dnmx_type_implements_strong(dnmx_type_handle ht, const dnmx_feature_info* info) {
    return self->implements_strong(*info);
}
bool dnmx_type_implements_strong_by_name(dnmx_type_handle ht, dnmx_sv name) {
    return self->implements_strong(name.to_std());
}

bool dnmx_type_implements(dnmx_type_handle ht, const dnmx_feature_info* info) {
    return self->implements(*info);
}

//dnmx_mixin_index_t dnmx_type_is_of() {}

bool dnmx_type_is_default_constructible(dnmx_type_handle ht) {
    return self->default_constructible();
}
bool dnmx_type_is_copy_constructible(dnmx_type_handle ht) {
    return self->copy_constructible();
}
bool dnmx_type_is_copy_assignable(dnmx_type_handle ht) {
    return self->copy_assignable();
}
bool dnmx_type_is_equality_comparable(dnmx_type_handle ht) {
    return self->equality_comparable();
}
bool dnmx_type_is_comparable(dnmx_type_handle ht) {
    return self->comparable();
}

}
