// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "../dnmx/object.h"
#include "object.hpp"
#include "domain.hpp"
#include "type.hpp"

using namespace dynamix;

extern "C" {

#define self object::from_c_handle(ho)

dnmx_object_handle dnmx_create_object_empty(dnmx_domain_handle hd) {
    try {
        auto obj = new object(*domain::from_c_handle(hd));
        return obj->to_c_hanlde();
    }
    catch (std::exception&) {
        return nullptr;
    }
}
dnmx_object_handle dnmx_create_object_default(dnmx_type_handle ht) {
    try {
        auto obj = new object(*type::from_c_handle(ht));
        return obj->to_c_hanlde();
    }
    catch (std::exception&) {
        return nullptr;
    }
}
dnmx_object_handle dnmx_create_object_copy(dnmx_const_object_handle ho) {
    try {
        auto obj = new object(object::from_c_handle(ho)->copy());
        return obj->to_c_hanlde();
    }
    catch (std::exception&) {
        return nullptr;
    }
}
dnmx_object_handle dnmx_create_object_usurp(dnmx_object_handle ho) {
    try {
        auto obj = new object(std::move(*object::from_c_handle(ho)));
        return obj->to_c_hanlde();
    }
    catch (std::exception&) {
        return nullptr;
    }
}
void dnmx_destroy_object(dnmx_object_handle ho) {
    delete self;
}

dnmx_error_return_t dnmx_object_copy_from(dnmx_object_handle ho, dnmx_const_object_handle source) {
    try {
        self->copy_from(*object::from_c_handle(source));
        return dnmx_result_success;
    }
    catch (std::exception&) {
        return -1;
    }
}

dnmx_domain_handle dnmx_object_get_domain(dnmx_const_object_handle ho) {
    return self->get_domain().to_c_hanlde();
}
dnmx_type_handle dnmx_object_get_type(dnmx_const_object_handle ho) {
    return ho->m_type;
}

dnmx_mixin_index_t dnmx_object_num_mixins(dnmx_const_object_handle ho) {
    return self->num_mixins();
}

bool dnmx_object_has(dnmx_const_object_handle ho, const dnmx_mixin_info* info) {
    return self->has(*info);
}
bool dnmx_object_has_by_name(dnmx_const_object_handle ho, dnmx_sv name) {
    return self->has(name.to_std());
}

const void* dnmx_object_get(dnmx_const_object_handle ho, const dnmx_mixin_info* info) {
    return self->get(*info);
}
const void* dnmx_object_get_by_name(dnmx_const_object_handle ho, dnmx_sv name) {
    return self->get(name.to_std());
}
const void* dnmx_object_get_at(dnmx_const_object_handle ho, dnmx_mixin_index_t index) {
    return self->get_at(index);
}
void* dnmx_object_get_mut(dnmx_object_handle ho, const dnmx_mixin_info* info) {
    return self->get(*info);
}
void* dnmx_object_get_mut_by_name(dnmx_object_handle ho, dnmx_sv name) {
    return self->get(name.to_std());
}
void* dnmx_object_get_mut_at(dnmx_object_handle ho, dnmx_mixin_index_t index) {
    return self->get_at(index);
}

bool dnmx_object_clear(dnmx_object_handle ho) {
    if (self->sealed()) return false;
    self->clear();
    return true;
}
dnmx_error_return_t dnmx_object_reset_type(dnmx_object_handle ho, dnmx_type_handle ht) {
    if (ht) {
        try {
            self->reset_type(*type::from_c_handle(ht));
        }
        catch (std::exception&) {
            return -1;
        }
    }
    else {
        self->clear();
    }

    return dnmx_result_success;
}

bool dnmx_object_is_empty(dnmx_const_object_handle ho) {
    return self->empty();
}

void dnmx_object_seal(dnmx_object_handle ho) {
    self->seal();
}

bool dnmx_object_is_sealed(dnmx_const_object_handle ho) {
    return self->sealed();
}

bool dnmx_object_equals(dnmx_const_object_handle ho, dnmx_const_object_handle b) {
    return self->equals(*object::from_c_handle(b));
}

dnmx_int_result dnmx_object_compare(dnmx_const_object_handle ho, dnmx_const_object_handle b) {
    dnmx_int_result ret;
    try {
        ret.result = self->compare(*object::from_c_handle(b));
        ret.error = dnmx_result_success;
    }
    catch (std::exception&) {
        ret.error = -1;
    }
    return ret;
}

}