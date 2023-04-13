// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-msg.hpp"
#include "bmm-common.hpp"

#include <dynamix/declare_domain.hpp>
#include <dynamix/define_domain.hpp>
#include <dynamix/mixin_info_data.hpp>
#include <dynamix/type_mutation.hpp>
#include <dynamix/mutate_to.hpp>
#include <dynamix/msg/func_traits.hpp>
#include <dynamix/msg/define_msg.hpp>

DYNAMIX_MAKE_FUNC_TRAITS(accumulate);
DYNAMIX_DEFINE_MSG(accumulate_msg, multicast, accumulate, void, (const dobject&, uint64_t&, uint32_t));

struct bench;
DYNAMIX_DEFINE_DOMAIN(bench, "bench");

std::vector<std::unique_ptr<dynamix::util::mixin_info_data>> mixin_infos;

using ctor = void(*)(void* buf, uint32_t a, uint32_t b);

template <typename Mixin>
void register_mixin(std::string_view name) {
    auto data = std::make_unique<dynamix::util::mixin_info_data>();
    dynamix::util::mixin_info_data_builder<Mixin> b(*data, name);
    b.template implements<accumulate_msg>();
    b.user_data(reinterpret_cast<uintptr_t>(static_cast<ctor>([](void* buf, uint32_t a, uint32_t b) {
        new (buf) Mixin(a, b);
    })));
    data->register_in(dynamix::g::get_domain<bench>());
    mixin_infos.push_back(std::move(data));
}

#define DEFINE_OP(name, code) \
    struct mop_##name { \
        uint32_t m_val_a, m_val_b; \
        mop_##name(uint32_t a, uint32_t b) : m_val_a(a), m_val_b(b) {} \
        void accumulate(uint64_t& acc, uint32_t arg) const { \
            code; \
        } \
    };

ITERATE_OPS(DEFINE_OP)

void register_ops() {
#define REGISTER_OP(name, code) register_mixin<mop_##name>(#name);
    ITERATE_OPS(REGISTER_OP)
}

dobject make_msg_accumulator(std::minstd_rand& rnd) {
    // lazy init
    if (mixin_infos.empty()) register_ops();

    const auto combo = get_ops_combo(rnd);

    auto& dom = dynamix::g::get_domain<bench>();
    dynamix::type_mutation tm(dom);
    for (auto& c : combo) {
        tm.add(mixin_infos[c.op]->info);
    }
    auto& type = dom.get_type(std::move(tm));

    dobject ret(dom);
    {
        dynamix::object_mutation om(ret, type);
        for (auto& c : combo) {
            om.update_next_mixin([&](dynamix::init_new_args args) {
                auto init = reinterpret_cast<ctor>(args.info.user_data);
                init(args.mixin_buf, c.a, c.b);
            }, dynamix::util::noop_udpate_common_func);
        }
    }

    return ret;
}
