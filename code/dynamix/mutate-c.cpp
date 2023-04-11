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

            auto wrap_init = [&](init_new_args args) {
                if (o->init_new) {
                    auto err = o->init_new({&args.info, args.mixin_buf, &args.target_type, nullptr, args.target_index, invalid_mixin_index, o->user_data});
                    if (err) {
                        throw_exception::obj_mut_user_error(args.target_type, "mutate_to", "init", args.info, err);
                    }
                }
                else {
                    util::default_init_new_func(args);
                }
            };

            auto wrap_update = [&](update_common_args args) {
                if (o->update_common) {
                    auto err = o->update_common({&args.info, args.mixin_buf, &args.target_type, &args.source_type, args.target_index, args.source_index, o->user_data});
                    if (err) {
                        throw_exception::obj_mut_user_error(args.target_type, "mutate_to", "update", args.info, err);
                    }
                }
                else {
                    util::noop_udpate_common_func(args);
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

            auto wrap_init = [&](init_new_args args) {
                auto err = o->init_override({&args.info, args.mixin_buf, &args.target_type, nullptr, args.target_index, invalid_mixin_index, o->user_data});
                if (err) {
                    throw_exception::obj_mut_user_error(t, "mutate", "init", args.info, err);
                }
            };

            obj_mut.update_at(i, wrap_init, util::noop_udpate_common_func);
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
