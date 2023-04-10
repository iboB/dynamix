// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "../dnmx/mutate.h"

#include "type.hpp"
#include "object.hpp"
#include "domain.hpp"
#include "object_mutation.hpp"
#include "error_return.hpp"
#include "type_mutation.hpp"
#include "throw_exception.hpp"

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
                if (ov.mixin) return ov.mixin == m;
                return ov.mixin_name == m->name;
            });
            if (!o) continue; // not overridden

            auto wrap_init = [&](const mixin_info& info, mixin_index_t index, byte_t* mixin) {
                if (o->init_new) {
                    auto err = o->init_new(&info, o->user_data, index, mixin);
                    if (err) {
                        throw_exception::obj_mut_user_error(t, "mutate_to", "init", info, err);
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
                        throw_exception::obj_mut_user_error(t, "mutate_to", "update", info, err);
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

DYNAMIX_API dnmx_error_return_t dnmx_mutate(dnmx_object_handle ho, const dnmx_mutate_op* pops, uint32_t num_mutate_ops) {
    try {
        itlib::span ops(pops, num_mutate_ops);
        auto* obj = object::from_c_handle(ho);
        type_mutation type_mut(obj->get_type());

        for (auto& op : ops) {
            switch (op.op_type) {
            case dnmx_mutate_op_add:
                if (op.mixin) type_mut.add(*op.mixin);
                else type_mut.add(op.mixin_name.to_std());
                break;
            case dnmx_mutate_op_remove:
                if (op.mixin) type_mut.remove(*op.mixin);
                else type_mut.remove(op.mixin_name.to_std());
                break;
            case dnmx_mutate_op_to_back:
                if (op.mixin) type_mut.to_back(*op.mixin);
                else type_mut.to_back(op.mixin_name.to_std());
                break;
            default:
                return -1;
            }
        }

        auto& t = obj->get_domain().get_type(std::move(type_mut));

        // mostly copy-pasta of code from uptade_to:
        object_mutation obj_mut(*obj, t);

        for (mixin_index_t i = 0; i < t.num_mixins(); ++i) {
            auto* m = t.mixins[i];
            auto o = itlib::pfind_if(ops, [&](const dnmx_mutate_op& op) {
                if (op.op_type != dnmx_mutate_op_add) return false; // only collect add ops
                if (!op.init_override) return false; // ... which have an init override
                // ... for the mixin in question
                if (op.mixin) return op.mixin == m;
                return op.mixin_name == m->name;
            });
            if (!o) continue; // not overridden

            auto wrap_init = [&](const mixin_info& info, mixin_index_t index, byte_t* mixin) {
                auto err = o->init_override(&info, o->user_data, index, mixin);
                if (err) {
                    throw_exception::obj_mut_user_error(t, "mutate", "init", info, err);
                }
            };

            obj_mut.update_at(i, wrap_init, object_mutation::empty_udpate_func);
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
