// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "mixin_info_fwd.hpp"
#include "mixin_index.hpp"
#include "size.hpp"
#include "globals.hpp"
#include "common_mixin_info.hpp"
#include "object_mutation_funcs.hpp"

#include "bits/make_from_tuple.hpp"

namespace dynamix {
class object;

struct DYNAMIX_API object_mutate_op {
protected:
    ~object_mutate_op() = default;
public:
    const mixin_info* affected_info = nullptr;
    mixin_index_t mixin_index = invalid_mixin_index;

    object_mutate_op* to_obj_mutate(object&) { return this; }

    virtual bool overrides_init() const noexcept; // true by default
    virtual void do_init(init_new_args args) = 0;
};

template <typename ConstructNew_Func>
struct object_mutate_op_with_func : public object_mutate_op {
    ConstructNew_Func func;
    object_mutate_op_with_func(ConstructNew_Func&& f) : func(std::forward<ConstructNew_Func>(f)) {}
    object_mutate_op_with_func(const mixin_info& info, ConstructNew_Func&& f) : func(std::forward<ConstructNew_Func>(f)) {
        affected_info = &info;
    }
    virtual bool overrides_init() const noexcept final override { return true; }
    virtual void do_init(init_new_args args) final override {
        func(args);
    }
};

template <typename Mixin, typename... Args>
struct object_mutate_op_with_args : public object_mutate_op {
    // with C++20 we can remove the tuple and switch to a variadic capture using add_mixin_op_with_func_by_info:
    // [args... = std::forward<Args>(args)...]
    // sadly we cannot do the easiest thing and capture args by ref: [&]
    // * if the args to add itself are (Args... args), there will be copies and references won't be propagated
    // * if args are refs (Args&... args), no temporaries can be used as args
    // * if args are rvalue refs like they are here, there will be temporary refs for temporaries
    //   which would be out of scope by the time the lambda is invoked

    std::tuple<Args...> args_tup;

    object_mutate_op_with_args(Args&&... args)
        : args_tup(std::forward<Args>(args)...)
    {
        this->affected_info = &g::get_mixin_info<Mixin>();
    }
    virtual bool overrides_init() const noexcept final override { return true; }
    virtual void do_init(init_new_args args) final override {
        impl::make_from_tuple<Mixin>(args.mixin_buf, std::move(args_tup));
    }
};

}
