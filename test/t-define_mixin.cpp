// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_mixin_allocator.hpp"

#include <dynamix/declare_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/domain_settings_builder.hpp>
#include <dynamix/globals.hpp>
#include <dynamix/domain.hpp>
#include <dynamix/object.hpp>
#include <dynamix/type.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/exception.hpp>
#include <dynamix/common_feature_info.hpp>
#include <dynamix/any.hpp>
#include <dynamix/common_mutation_rules.hpp>
#include <dynamix/common_mixin_init.hpp>
#include <dynamix/declare_type_class.hpp>

#include <dynamix/compat/pmr/string.hpp>

#include <doctest/doctest.h>
#include <doctest/util/lifetime_counter.hpp>

TEST_SUITE_BEGIN("dynamix");

DYNAMIX_DECLARE_DOMAIN(test);

DYNAMIX_DECLARE_MIXIN(struct alice);
DYNAMIX_DECLARE_MIXIN(class bob);
DYNAMIX_DECLARE_MIXIN(class charlie);

DYNAMIX_DECLARE_TYPE_CLASS(impl_bbb);
DYNAMIX_DECLARE_TYPE_CLASS(impl_ccc);

int domain_ctx;

struct alice_member : public doctest::util::lifetime_counter<alice_member> {
    int i = 5;
};

TEST_CASE("domain") {
    auto& d = dynamix::g::get_domain<test>();
    CHECK(d.name() == "test");
    CHECK(d.settings().canonicalize_types);
    CHECK_FALSE(d.settings().allow_duplicate_feature_names);
    CHECK_FALSE(d.settings().allow_duplicate_mixin_names);
    CHECK(d.user_data == 232);
    CHECK(d.context == &domain_ctx);
}

class test_obj : public dynamix::object {
public:
    test_obj() : dynamix::object(dynamix::g::get_domain<test>()) {}
};

template <typename NameFeature>
struct name_feature_traits;

struct feature_aaa {
    using traits = name_feature_traits<feature_aaa>;
    static const dynamix::common_feature_info& get_info_safe();
    static const dynamix::feature_info& info;
};

struct feature_bbb {
    using traits = name_feature_traits<feature_bbb>;
    static const dynamix::common_feature_info& get_info_safe();
    static const dynamix::feature_info& info;
};

struct feature_ccc {
    using traits = name_feature_traits<feature_ccc>;
    static const dynamix::common_feature_info& get_info_safe();
    static const dynamix::feature_info& info;
};

struct feature_ddd {
    using traits = name_feature_traits<feature_ddd>;
    static const dynamix::common_feature_info& get_info_safe();
    static const dynamix::feature_info& info;
};

template <typename F>
dynamix::compat::pmr::string& get_string(test_obj& object) {
    auto& info = dynamix::g::get_feature_info_fast<F>();
    auto fe = object.get_type().ftable_at(info.id); // ftable entry

    void* data = nullptr;
    if (fe) {
        data = fe.begin->payload;
    }
    else {
        data = info.default_payload;
    }

    if (!data) throw dynamix::bad_feature_access("test string");

    return *reinterpret_cast<dynamix::compat::pmr::string*>(data);
}

struct mixin_charlie : public dynamix::common_mixin_init<charlie> {
    const char* lit;
    mixin_charlie(const char* lit) : lit(lit) {}
    virtual void do_init(dynamix::init_new_args args) final override;
};

TEST_CASE("declared mixins only") {
    alice_member::lifetime_stats as;
    doctest::util::lifetime_counter_sentry _l(as);

    test_obj obj;
    CHECK(obj.empty());
    CHECK(obj.get_type().implements<feature_ccc>());
    CHECK(get_string<feature_ccc>(obj) == "asdf");

    using namespace dynamix::mutate_ops;
    mutate(obj, add<alice>(), add("bob"));
    CHECK(obj.is_of<impl_ccc>());
    CHECK(obj.is_of("impl_ccc"));
    CHECK_FALSE(obj.is_of<impl_bbb>());
    CHECK_FALSE(obj.is_of("impl_bbb"));
    auto& t_a_b = obj.get_type();
    {
        auto& t = t_a_b;
        CHECK(t.num_mixins() == 3);
        CHECK(t.has("alice"));
        CHECK(t.has<bob>());
        CHECK(t.has("to Bob"));
        CHECK(t.lacks<charlie>());

        CHECK(t.implements_strong<feature_aaa>());
        CHECK(t.implements<feature_aaa>());
        CHECK_FALSE(t.implements_strong<feature_bbb>());
        CHECK_FALSE(t.implements<feature_bbb>());
        CHECK(t.implements_strong<feature_ccc>());
        CHECK(t.implements<feature_ccc>());
        CHECK(t.implements_strong<feature_ddd>());

        CHECK(t.is_of<impl_ccc>());
        CHECK(t.is_of("impl_ccc"));
        CHECK_FALSE(t.is_of<impl_bbb>());
        CHECK_FALSE(t.is_of("impl_bbb"));
    }

    CHECK(get_string<feature_aaa>(obj) == "alice-aaa");
    CHECK_THROWS_WITH_AS(get_string<feature_bbb>(obj), "test string", dynamix::bad_feature_access);
    CHECK(get_string<feature_ccc>(obj) == "bob-ccc");
    CHECK(get_string<feature_ddd>(obj) == "to Bob-ddd");

    CHECK(as.living == 1);
    CHECK(as.total == 1);

    mutate(obj, remove<bob>());
    {
        auto& t = obj.get_type();
        CHECK(t.num_mixins() == 1);
        CHECK(t.has<alice>());

        CHECK(t.implements_strong<feature_aaa>());
        CHECK(t.implements<feature_aaa>());
        CHECK_FALSE(t.implements_strong<feature_bbb>());
        CHECK_FALSE(t.implements<feature_bbb>());
        CHECK(t.implements_strong<feature_ccc>());
        CHECK(t.implements<feature_ccc>());
        CHECK_FALSE(t.implements_strong<feature_ddd>());
        CHECK_FALSE(t.implements<feature_ddd>());
    }

    CHECK(get_string<feature_aaa>(obj) == "alice-aaa");
    CHECK(get_string<feature_ccc>(obj) == "alice-ccc");

    CHECK(as.living == 1);
    CHECK(as.total == 1);

    auto ntypes = dynamix::g::get_domain<test>().num_types();
    auto nqueries = dynamix::g::get_domain<test>().num_type_queries();
    dynamix::mutate(obj)
        .add<bob>()
        .to_back<alice>();
    {
        auto& t = obj.get_type();
        CHECK(t.num_mixins() == 3);
        CHECK(&t == &t_a_b); // we have canonical order
    }
    CHECK(dynamix::g::get_domain<test>().num_types() == ntypes);
    CHECK(dynamix::g::get_domain<test>().num_type_queries() == nqueries + 1);

    CHECK_THROWS_WITH_AS(dynamix::mutate(obj, dynamix::add<charlie>()), "missing default init", dynamix::mutation_error);

    CHECK(as.living == 1);
    CHECK(as.total == 1);

    test_obj obj2;
    obj2.copy_from(obj);

    auto o2bob = obj2.get<bob>();
    CHECK(get_string<feature_aaa>(obj2) == "alice-aaa");
    CHECK(get_string<feature_ccc>(obj2) == "bob-ccc");

    auto obj3 = std::move(obj2);
    CHECK(obj2.empty());
    CHECK(obj3.get<bob>() == o2bob);

    {
        const auto& cobj3 = obj3;
        CHECK(cobj3.has<bob>());
        CHECK(cobj3.get<bob>() == o2bob);
    }

    CHECK(as.living == 2);
    CHECK(as.d_ctr == 1);
    CHECK(as.c_ctr == 1);
    CHECK(as.total == 2);
}

void test_charlie_common_mixin_init(test_obj& obj);

TEST_CASE("common mixin init") {
    test_obj obj;

    mutate(obj, dynamix::add<alice>(), mixin_charlie("horse"));
    CHECK(obj.has<alice>());
    CHECK(obj.has<charlie>());
    test_charlie_common_mixin_init(obj);

    CHECK(obj.is_of<impl_bbb>());
    CHECK(obj.is_of("impl_bbb"));
}

// definitions

#include <dynamix/define_type_class.hpp>

bool implements_bbb(const dnmx_type_handle th) noexcept {
    return dynamix::type::from_c_handle(th)->implements_strong<feature_bbb>();
}
DYNAMIX_DEFINE_TYPE_CLASS_WITH(test, impl_bbb, implements_bbb);
bool implements_ccc(const dnmx_type_handle th) noexcept {
    return dynamix::type::from_c_handle(th)->implements_strong<feature_ccc>();
}
DYNAMIX_DEFINE_TYPE_CLASS_WITH(test, impl_ccc, implements_ccc);

struct alice {
    alice() = default;
    alice(alice_member a) : m(std::move(a)) {}
    alice_member m;
};

class bob {
public:
    float x, y, z;
};

class charlie : public doctest::util::lifetime_counter<charlie> {
public:
    explicit charlie(std::string_view sv) : str(sv) {}
    explicit charlie(std::vector<int> ints) : vec(std::move(ints)) {}
    std::string str;
    std::vector<int> vec;
};

void test_charlie_common_mixin_init(test_obj& obj) {
    CHECK(obj.get<charlie>()->str == "horse");
}

class donna {
public:
    int i = 10;
    donna() = default;
    explicit donna(int i) : i(i) {};
    donna(int a, int b) : i(a + b) {};
};

TEST_CASE("mutate construct") {
    using namespace dynamix::mutate_ops;

    {
        test_obj obj;
        mutate(obj
            , add<alice>()
            , add<charlie>("cha")
        );

        CHECK(obj.get<charlie>());
        CHECK(obj.get<charlie>()->str == "cha");
    }

    {
        std::vector<int> ints = {1, 2, 3};
        auto ints_data = ints.data();
        test_obj obj;
        mutate(obj
            , add<charlie>(std::move(ints))
            , add<donna>()
            , add<alice>()
        );

        CHECK(obj.get<donna>()->i == 10);

        auto& cints = obj.get<charlie>()->vec;
        CHECK(cints.size() == 3);
        CHECK(cints.data() == ints_data);
    }

    const dynamix::type* tcda = nullptr;
    {
        test_obj obj;
        mutate(obj
            , add<charlie>(std::initializer_list<int>{5, 6, 7, 8})
            , add<donna>(1, 2)
            , add<alice>()
        );

        CHECK(obj.get<donna>()->i == 3);
        CHECK(obj.get<charlie>()->vec == std::vector<int>{5, 6, 7, 8});

        tcda = &obj.get_type();
    }

    {
        using namespace dynamix::mutate_to_ops;
        test_obj obj;
        mutate_to(obj, *tcda,
            construct<donna>(5, 6),
            construct<charlie>("asdf")
        );

        CHECK(obj.get<donna>()->i == 11);
        CHECK(obj.get<charlie>()->str == "asdf");
    }
}

class to_bob {};

#include <dynamix/define_mixin.hpp>

template <typename NameFeature>
struct name_feature_traits {
    static dynamix::any make_payload_for(::dynamix::allocator alloc, const dynamix::feature_info& f, const dynamix::mixin_info& m) {
        dynamix::compat::pmr::string str(m.name.to_std(), alloc);
        str += '-';
        str += f.name.to_std();
        auto ret = dynamix::make_any(alloc, std::move(str));
        return ret;
    }

    template <typename M>
    static dynamix::any make_payload_for(::dynamix::allocator a, M*) {
        return make_payload_for(a, NameFeature::get_info_safe(), dynamix::g::get_mixin_info<M>());
    }

    static dynamix::any make_payload_by(::dynamix::allocator, void*, std::string& str) {
        return dynamix::fwd_any(str);
    }

    template <typename PL>
    static dynamix::any make_payload_with(::dynamix::allocator alloc, void*, PL&& str) {
        return dynamix::make_any(alloc, dynamix::compat::pmr::string(std::forward<PL>(str), alloc));
    }
};

DYNAMIX_DEFINE_MIXIN(test, alice)
    .uses_allocator<test_mixin_allocator>()
    .implements<feature_aaa>(5_bid)
    .implements<feature_ccc>()
    ;
DYNAMIX_DEFINE_MIXIN(test, bob)
    .implements<feature_aaa>()
    .implements<feature_ccc>()
    .implements<feature_ddd>();
    ;
DYNAMIX_DEFINE_MIXIN(test, charlie)
    .implements<feature_aaa>()
    .implements<feature_bbb>()
    ;

void mixin_charlie::do_init(dynamix::init_new_args args) {
    new (args.mixin_buf) charlie(lit);
}

DYNAMIX_DEFINE_MIXIN(test, to_bob)
    .name("to Bob")
    .on_mutation(attaches_to<bob>{}, 4_prio)
    .implements<feature_ddd>(-10_prio);
DYNAMIX_DEFINE_MIXIN(test, donna);

#include <dynamix/define_domain.hpp>

DYNAMIX_DEFINE_DOMAIN(test,
    "test",
    dynamix::domain_settings_builder{}.canonicalize_types(true).allow_duplicate_feature_names(false),
    232,
    &domain_ctx
);

const dynamix::common_feature_info& feature_aaa::get_info_safe() {
    static dynamix::common_feature_info fi(dnmx_make_sv_lit("aaa"), true);
    return fi;
}
const dynamix::feature_info& feature_aaa::info = feature_aaa::get_info_safe();

const dynamix::common_feature_info& feature_bbb::get_info_safe() {
    static dynamix::common_feature_info fi(dnmx_make_sv_lit("bbb"));
    return fi;
}
const dynamix::feature_info& feature_bbb::info = feature_bbb::get_info_safe();

const dynamix::common_feature_info& feature_ccc::get_info_safe() {
    static dynamix::compat::pmr::string dpl = "asdf";
    static dynamix::common_feature_info fi(dnmx_make_sv_lit("ccc"), true, &dpl);
    return fi;
}
const dynamix::feature_info& feature_ccc::info = feature_ccc::get_info_safe();

const dynamix::common_feature_info& feature_ddd::get_info_safe() {
    static dynamix::common_feature_info fi(dnmx_make_sv_lit("ddd"), true);
    return fi;
}
const dynamix::feature_info& feature_ddd::info = feature_ddd::get_info_safe();
