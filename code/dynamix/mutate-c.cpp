// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "../dnmx/mutate.h"

#include "type.hpp"
#include "object.hpp"
#include "object_mutation.hpp"
#include "error_return.hpp"
#include "exception.hpp"

#include <itlib/qalgorithm.hpp>

using namespace dynamix;

#if defined(__cplusplus)
extern "C" {
#endif

dnmx_error_return_t dnmx_mutate_to(dnmx_object_handle obj, dnmx_type_handle ht, const dnmx_mutate_to_override* po, uint32_t num_overrides) {
    try {
        auto& t = *type::from_c_handle(ht);
        object_mutation mut(*object::from_c_handle(obj), t);

        itlib::span overrides(po, num_overrides);
        for (mixin_index_t i = 0; i < t.num_mixins(); ++i) {
            auto* m = t.mixins[i];
            auto o = itlib::pfind_if(overrides, [&](const dnmx_mutate_to_override& ov) {
                return ov.mixin == m;
            });
            if (!o) continue; // not overridden

            auto wrap_init = [&](const mixin_info& info, mixin_index_t index, byte_t* mixin) {
                if (o->init_new) {
                    auto err = o->init_new(&info, o->user_data, index, mixin);
                    if (err) {
                        throw mutation_user_error("mutate_to", err);
                    }
                }
                else {
                    object_mutation::default_init_mixin(info, index, mixin);
                }
            };

            auto wrap_update = [&](const mixin_info& info, mixin_index_t index, byte_t* mixin) {
                if (o->update_common) {
                    auto err = o->update_common(&info, o->user_data, index, mixin);
                    if (err) {
                        throw mutation_user_error("mutate_to", err);
                    }
                }
                else {
                    object_mutation::empty_udpate_func(info, index, mixin);
                }
            };

            mut.update_at(i, wrap_init, wrap_update);
        }

        return result_success;
    }
    catch (std::exception&) {
        return -1;
    }
}

#if defined(__cplusplus)
}
#endif
