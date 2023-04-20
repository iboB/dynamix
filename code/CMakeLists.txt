# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
include(icm_add_lib)

icm_add_lib(dynamix DYNAMIX)
add_library(dynamix::dynamix ALIAS dynamix)
target_include_directories(dynamix INTERFACE .)

CPMAddPackage("gh:iboB/itlib@1.9.0")
CPMAddPackage("gh:iboB/splat@1.3.1")

target_link_libraries(dynamix PUBLIC
    splat::splat
    itlib::itlib
)

target_sources(dynamix PRIVATE
    dnmx/api.h

    dnmx/bits/empty_init.h
    dnmx/bits/id_int_t.h
    dnmx/bits/no_sanitize.h
    dnmx/bits/noexcept.h
    dnmx/bits/pp.h
    dnmx/bits/sv.h

    dynamix/bits/make_from_tuple.hpp
    dynamix/bits/make_nullptr.hpp
    dynamix/bits/q_const.hpp
    dynamix/bits/type_name_from_typeid.hpp

    dynamix/compat/pmr/pmr.hpp
    dynamix/compat/pmr/allocator.hpp
    dynamix/compat/pmr/deque.hpp
    dynamix/compat/pmr/memory_resource.hpp
    dynamix/compat/pmr/memory_resource.cpp
    dynamix/compat/pmr/string.hpp
    dynamix/compat/pmr/vector.hpp

    dnmx/size.h
    dynamix/size.hpp
    dnmx/error_return.h
    dynamix/error_return.hpp

    dnmx/feature_id.h
    dynamix/feature_id.hpp
    dnmx/feature_payload.h
    dynamix/feature_payload.hpp
    dnmx/feature_info.h
    dynamix/feature_info_fwd.hpp
    dynamix/feature_info.hpp
    dynamix/common_feature_info.hpp
    dynamix/feature_info_data.hpp

    dnmx/mixin_id.h
    dynamix/mixin_id.hpp
    dnmx/mixin_info.h
    dnmx/mixin_info_util.h
    dnmx/mixin_common_funcs.h
    dnmx/mixin_common_funcs.c
    dynamix/mixin_info_fwd.hpp
    dynamix/mixin_info.hpp
    dynamix/mixin_info_util.hpp
    dynamix/common_mixin_info.hpp
    dynamix/mixin_info_data.hpp
    dynamix/mixin_info_data.cpp
    dynamix/mixin_info_data_literals.hpp

    dnmx/mixin_allocator.h
    dnmx/mixin_allocator_handle.h
    dynamix/mixin_allocator.hpp
    dynamix/mixin_allocator-c.cpp

    dnmx/feature_for_mixin.h
    dynamix/feature_for_mixin_fwd.hpp
    dynamix/feature_for_mixin.hpp

    dnmx/mutation_rule_info.h
    dynamix/mutation_rule_info_fwd.hpp
    dynamix/mutation_rule_info.hpp

    dynamix/alloc_util.hpp
    dynamix/allocator.hpp
    dynamix/any.hpp

    dnmx/domain_settings.h
    dnmx/basic_domain.h
    dynamix/domain_settings.hpp
    dynamix/domain_settings_builder.hpp
    dynamix/domain.hpp
    dynamix/domain.cpp
    dnmx/domain.h
    dnmx/domain_handle.h
    dynamix/domain-c.cpp
    dynamix/domain.from_info.hpp
    dynamix/domain_traverse.hpp

    dnmx/object_mixin_data.h
    dynamix/object_mixin_data.hpp
    dnmx/basic_object.h
    dynamix/object.hpp
    dynamix/object.cpp
    dynamix/object-c.cpp
    dnmx/object_handle.h
    dynamix/object_of.hpp

    dnmx/mixin_index.h
    dynamix/mixin_index.hpp
    dnmx/basic_type.h
    dynamix/type.hpp
    dynamix/type.cpp
    dnmx/type.h
    dnmx/type_handle.h
    dynamix/type-c.cpp

    dynamix/exception.hpp
    dynamix/exception.cpp
    dynamix/throw_exception.hpp
    dynamix/throw_exception.cpp

    dnmx/type_class.h
    dynamix/type_class.hpp
    dynamix/declare_type_class.hpp
    dynamix/define_type_class.hpp

    dynamix/type_mutation.hpp
    dynamix/type_mutation.cpp
    dnmx/type_mutation.h
    dnmx/type_mutation_handle.h
    dynamix/type_mutation-c.cpp
    dynamix/object_mutation_funcs.hpp
    dynamix/object_mutation.hpp
    dynamix/object_mutation.cpp
    dynamix/object_mutate_ops.hpp
    dynamix/object_mutate_ops.cpp
    dynamix/mutate.hpp
    dynamix/mutate_ops.hpp
    dynamix/mutate_to.hpp
    dynamix/mutate_to_ops.hpp
    dnmx/mutate.h
    dynamix/mutate-c.cpp

    dynamix/dbg_dmp.hpp
    dynamix/dbg_dmp.cpp

    dynamix/common_mixin_init.hpp

    dynamix/common_mutation_rules.hpp

    dynamix/globals.hpp

    dynamix/dm_this.hpp
    dynamix/declare_domain.hpp
    dynamix/declare_mixin.hpp
    dynamix/define_domain.hpp
    dynamix/define_mixin.hpp

    dnmx/msg/msg_macros.h
)
