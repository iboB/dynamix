// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "type.hpp"
#include "object_mixin_data.hpp"
#include "mixin_info.hpp"
#include "globals.hpp"

namespace dynamix {
class object;

class DYNAMIX_API object_mutation {
public:
    // init for a transition to a type
    // may thow if the related allocations throw
    object_mutation(object& obj, const type& type);

    // if no exceptions are thrown, will try to complete the mutation by default_construct_each_new_mixin
    ~object_mutation() noexcept(false);

    object_mutation(const object_mutation&) = delete;
    object_mutation& operator=(const object_mutation&) = delete;
    object_mutation(object_mutation&&) noexcept = delete; // allow factory constructors
    object_mutation& operator=(object_mutation&&) = delete;

    static void empty_udpate_func(const mixin_info&, mixin_index_t, byte_t*) {}

    // update next mixin (in order of construction) after the one reached by the last piecewise call
    //
    // * call construct_new if the mixin is new and assume it's constructed after the call
    // void construct_new(const mixin_info& info, mixin_index_t new_index, byte_t* new_mixin)
    // DANGER: don't return if the mixin is not constructed! to abort, throw an exception.
    //
    // * call update_common if the mixin is common
    // void update_common(const mixin_info& info, mixin_index_t new_index, byte_t* common_mixin)
    template <typename ConstructNew_Func, typename UpdateCommon_Func>
    void update_next_mixin(ConstructNew_Func&& construct_new, UpdateCommon_Func&& update_common) {
        if (m_complete) return;
        auto ti = m_updated_upto;
        const auto& info = *m_target_type.mixins[ti];
        if (m_old_mixin_data && m_old_type->has(info.id)) {
            // matching
            auto oi = m_old_type->sparse_mixin_indices[info.iid()];
            auto old_mixin = m_old_mixin_data[oi].mixin;
            update_common(info, ti, old_mixin);
        }
        else {
            // new
            construct_new(info, ti, m_target_mixin_data[ti].mixin);
        }
        ++m_updated_upto;
        m_complete = (m_updated_upto == m_target_type.num_mixins());
    }

    // update object mixins in order of construction from the one reached by the last piecewise call
    // to the one provided as an upto_index argument
    template <typename ConstructNew_Func, typename UpdateCommon_Func>
    void update_each_mixin(ConstructNew_Func&& construct_new, UpdateCommon_Func&& update_common, mixin_index_t upto_index = invalid_mixin_index) {
        // don't overflow
        if (upto_index > m_target_type.num_mixins()) upto_index = m_target_type.num_mixins();
        while (m_updated_upto < upto_index) {
            update_next_mixin(construct_new, update_common); // pass functors by const ref
        }
    }

    // construct new mixins and don't touch common ones
    // will count common mixins as updated
    // (calls update_mixins with construct_new and noop)
    template <typename ConstructNew_Func>
    void construct_each_new_mixin(ConstructNew_Func&& construct_new, mixin_index_t upto_index = invalid_mixin_index) {
        update_each_mixin(std::forward<ConstructNew_Func>(construct_new),
            empty_udpate_func,
            upto_index);
    }

    // calls default init for each new mixins and counts the common ones as updated
    void default_construct_each_new_mixin(mixin_index_t upto_index = invalid_mixin_index);

    // update mixin at given index (piecewise mutation)
    // default construct ones between the last piecewise call and the provided index
    // WARNING: this must be called in the order of the mixins in the object
    // with domain_settings::canonicalize_types set to true, this might get tricky
    template <typename ConstructNew_Func, typename UpdateCommon_Func>
    void update_at(mixin_index_t index, ConstructNew_Func&& construct_new, UpdateCommon_Func&& update_common) {
        if (index < m_updated_upto || index > m_target_type.num_mixins()) {
            throw_bad_piecewise_mutation();
        }
        default_construct_each_new_mixin(index);
        update_next_mixin(std::forward<ConstructNew_Func>(construct_new), std::forward<UpdateCommon_Func>(update_common));
    }

    //////////////////////////////////////////////////

    // check if the mutation is complete
    // called by the destructor to determine whether to finalize or rollback
    // there is rarely a need to call it manually
    [[nodiscard]] bool complete() const noexcept { return m_complete; }

    // will apply changes to object and make it valid
    // this is called in the destructor if the mutation is complete
    // there is rarely a need to call it manually
    void finalize() noexcept;

    // will undo all changes made up to this point
    // this is called in the destructor if the mutation is not complete
    // there is rarely a need to call it manually
    void rollback() noexcept;

private:
    friend class object;

    // init for a transition to a type
    // delegated to by the public constructor
    object_mutation(object& obj, const type& type, int) noexcept;

    // "inverse" init
    // will fill target values from object
    // this is useful if one only wans to call the destroy and deallocate functions
    object_mutation(std::nullptr_t, object& obj) noexcept;

    // moved to a function so that we don't have to include exception.hpp here
    [[noreturn]] void throw_bad_piecewise_mutation();

    // object being mutated
    object& m_object;

    // object's old type and mixin data
    // can both be null or if m_old_type is the empty type, m_old_mixin_data will be null
    // if m_old_mixin_data is not null, m_old_type must be a valid type
    const type* m_old_type;
    object_mixin_data* m_old_mixin_data;

    // the target type and mixin data
    // the target type cannot be null
    const type& m_target_type;
    object_mixin_data* m_target_mixin_data;

    // rollback helpers
    // one after the last target_type index for which:

    // ... new mixins which needed allocation were allocated
    mixin_index_t m_allocated_upto;

    // ... mixins for target_type have been updated
    // * target-only mixins have been constructed
    // * common mixins have been updated
    mixin_index_t m_updated_upto;

    // prepare the unified buffer (mixin data) for the target_object type
    void allocate_mixin_data();
    void deallocate_mixin_data() noexcept;
    [[nodiscard]] byte_t* allocate_external_mixin(const mixin_info& info);
    void deallocate_external_mixin(void* ptr, const mixin_info& info) noexcept;

    // destroy target-only mixins mixin data
    void destroy_new_mixins() noexcept;

    bool m_complete = false;

    // rust-style bool to help us support moving mutations
    bool m_moved_out_from = false;
};

// utility funcs for the most common use cases:

template <typename ConstructNew_Func, typename UpdateCommon_Func>
void perform_object_mutation(object& obj, const type& target_type, ConstructNew_Func&& construct_new, UpdateCommon_Func&& update_common) {
    object_mutation mut(obj, target_type);
    mut.update_each_mixin(std::forward<ConstructNew_Func>(construct_new), std::forward<UpdateCommon_Func>(update_common));
}

template <typename ConstructNew_Func>
void perform_object_mutation(object& obj, const type& target_type, ConstructNew_Func&& construct_new) {
    object_mutation mut(obj, target_type);
    mut.construct_each_new_mixin(std::forward<ConstructNew_Func>(construct_new));
}

inline void perform_object_mutation(object& obj, const type& target_type) {
    object_mutation mut(obj, target_type);
    mut.default_construct_each_new_mixin();
}

}
