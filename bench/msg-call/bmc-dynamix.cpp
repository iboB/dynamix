// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmc-common.hpp"

#include <dynamix/define_domain.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/define_type_class.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>
#include <dynamix/msg/next_impl.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/type.hpp>

DYNAMIX_DEFINE_DOMAIN(bcall);

DYNAMIX_MAKE_FUNC_TRAITS(get_value);
DYNAMIX_MAKE_FUNC_TRAITS(ident);
DYNAMIX_MAKE_FUNC_TRAITS(ident_default);
DYNAMIX_MAKE_FUNC_TRAITS(tag);

int value_holder::get_value(int arg) const {
    return v + arg;
}
int value_holder::ident(int arg) const {
    return arg;
}
DYNAMIX_DEFINE_MIXIN(bcall, value_holder)
    .implements<get_value_msg>()
    .implements<ident_msg>()
;

int tagged::tag() const {
    return 1;
}
DYNAMIX_DEFINE_MIXIN(bcall, tagged)
    .implements<tag_msg>()
;

// decorates get_value by calling the next implementation
struct decorator {
    int get_value(int arg) const {
        return DYNAMIX_CALL_NEXT_IMPL_MSG(get_value_msg, arg);
    }
};
DYNAMIX_DECLARE_MIXIN(decorator);
DYNAMIX_DEFINE_MIXIN(bcall, decorator)
    .implements<get_value_msg>(feature_bid(1))
;

// filler mixins, each with a message of its own
// they make the objects and their feature tables bigger
#define FILLER(n) \
    DYNAMIX_DECLARE_MSG(filler_msg_##n, filler_##n, int, (const dynamix::object&)); \
    DYNAMIX_MAKE_FUNC_TRAITS(filler_##n); \
    DYNAMIX_DEFINE_MSG(filler_msg_##n, unicast, filler_##n, int, (const dynamix::object&)); \
    struct filler_mixin_##n { \
        int data = n; \
        int filler_##n() const { return data; } \
    }; \
    DYNAMIX_DECLARE_MIXIN(filler_mixin_##n); \
    DYNAMIX_DEFINE_MIXIN(bcall, filler_mixin_##n).implements<filler_msg_##n>()

FILLER(0); FILLER(1); FILLER(2); FILLER(3); FILLER(4);
FILLER(5); FILLER(6); FILLER(7); FILLER(8);

namespace {
int default_ident(const dynamix::object&, int arg) {
    return arg;
}
}

DYNAMIX_DEFINE_MSG(get_value_msg, unicast, get_value, int, (const dynamix::object&, int));
DYNAMIX_DEFINE_MSG(ident_msg, unicast, ident, int, (const dynamix::object&, int));
DYNAMIX_DEFINE_MSG_EX(ident_default_msg, unicast, true, default_ident, ident_default, int, (const dynamix::object&, int));
DYNAMIX_DEFINE_MSG(tag_msg, unicast, tag, int, (const dynamix::object&));

DYNAMIX_DEFINE_TYPE_CLASS_WITH(bcall, is_tagged, [](const dynamix::type& t) { return t.has<tagged>(); });

namespace {
void mutate_to_base(dynamix::object& obj, int v) {
    using namespace dynamix;
    mutate(obj,
        add<filler_mixin_0>(), add<filler_mixin_1>(), add<filler_mixin_2>(), add<filler_mixin_3>(), add<filler_mixin_4>(),
        add<value_holder>(value_holder{v}),
        add<filler_mixin_5>(), add<filler_mixin_6>(), add<filler_mixin_7>(), add<filler_mixin_8>()
    );
}
}

dynamix::object make_obj(int v) {
    dynamix::object ret(dynamix::g::get_domain<bcall>());
    mutate_to_base(ret, v);
    return ret;
}

dynamix::object make_obj_with_decorator(int v) {
    auto ret = make_obj(v);
    dynamix::mutate(ret, dynamix::add<decorator>());
    return ret;
}

dynamix::object make_tagged_obj(int v) {
    auto ret = make_obj(v);
    dynamix::mutate(ret, dynamix::add<tagged>());
    return ret;
}
