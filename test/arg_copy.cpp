// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#include "doctest/doctest.h"

TEST_SUITE("arg copy");

enum track_tag
{
    COPY = 1,
    MOVE,
    REF,
};

template <track_tag Tag>
struct track
{
    track()
    {
        ++defaults;
    }

    track(const track& t)
        : val(t.val)
    {
        ++copies;
    }

    track(track&& t)
        : val(t.val)
    {
        t.val = -1;
        ++moves;
    }

    track& operator=(const track&) = delete;
    track& operator=(track&&) = delete;

    int val = Tag;

    static int defaults;
    static int copies;
    static int moves;

    static void reset()
    {
        defaults = copies = moves = 0;
    }
};

template <track_tag Tag>
int track<Tag>::defaults = 0;
template <track_tag Tag>
int track<Tag>::copies = 0;
template <track_tag Tag>
int track<Tag>::moves = 0;

using track_copy = track<COPY>;
using track_move = track<MOVE>;
using track_ref = track<REF>;

void reset_tracks()
{
    track_copy::reset();
    track_move::reset();
    track_ref::reset();
}

DYNAMIX_MESSAGE_3(int, uni, track_copy, cp, track_move&&, mv, track_ref&, ref);
DYNAMIX_MESSAGE_0(track_move, uni_ret);

DYNAMIX_CONST_MESSAGE_0(int, geta);
DYNAMIX_CONST_MESSAGE_0(int, getb);
DYNAMIX_CONST_MESSAGE_0(int, getc);

DYNAMIX_MULTICAST_MESSAGE_2(void, multi, track_copy, cp, track_ref&, ref);

DYNAMIX_DECLARE_MIXIN(a);
DYNAMIX_DECLARE_MIXIN(b);
DYNAMIX_DECLARE_MIXIN(c);

TEST_CASE("unicast")
{
    using namespace dynamix;
    object o;
    mutate(o).add<a>();

    track_copy c;
    track_move m;
    track_ref r;

    CHECK(track_copy::defaults == 1);
    CHECK(track_copy::copies == 0);
    CHECK(track_copy::moves == 0);
    CHECK(track_move::defaults == 1);
    CHECK(track_move::copies == 0);
    CHECK(track_move::moves == 0);
    CHECK(track_ref::defaults == 1);
    CHECK(track_ref::copies == 0);
    CHECK(track_ref::moves == 0);

    int n = uni(o, c, std::move(m), r);
    CHECK(n == 5);

    CHECK(track_copy::defaults == 1); // no change
    CHECK(track_copy::copies == 1); // argument to msg
    CHECK(track_copy::moves == 2); // move from msg to caller, then from caller to method
    CHECK(track_move::defaults == 1); // no change
    CHECK(track_move::copies == 0); // no change
    CHECK(track_move::moves == 1); // single actual move in method
    CHECK(track_ref::defaults == 1); // no change
    CHECK(track_ref::copies == 0); // no change
    CHECK(track_ref::moves == 0); // no change

    CHECK(geta(o) == -1);

    reset_tracks();
}

TEST_CASE("ret")
{
    using namespace dynamix;
    object o;
    mutate(o).add<a>();

    CHECK(track_copy::defaults == 0);
    CHECK(track_copy::copies == 0);
    CHECK(track_copy::moves == 0);
    CHECK(track_move::defaults == 0);
    CHECK(track_move::copies == 0);
    CHECK(track_move::moves == 0);
    CHECK(track_ref::defaults == 0);
    CHECK(track_ref::copies == 0);
    CHECK(track_ref::moves == 0);

    auto m = uni_ret(o);

    CHECK(track_copy::defaults == 0);
    CHECK(track_copy::copies == 0);
    CHECK(track_copy::moves == 0);
    CHECK(track_move::defaults == 1); // in the method
    CHECK(track_move::copies == 0);

#if defined(NDEBUG)
    // some moves get elided with optimizations
    CHECK(track_move::moves < 3);
#else
    // some moves get elided even without optimizations
    CHECK(track_move::moves < 5);
#endif

    CHECK(track_ref::defaults == 0);
    CHECK(track_ref::copies == 0);
    CHECK(track_ref::moves == 0);

    reset_tracks();
}

TEST_CASE("multi")
{
    using namespace dynamix;
    object o;
    mutate(o)
        .add<a>()
        .add<b>()
        .add<c>();

    track_copy c;
    track_ref r;

    CHECK(track_copy::defaults == 1);
    CHECK(track_copy::copies == 0);
    CHECK(track_copy::moves == 0);
    CHECK(track_ref::defaults == 1);
    CHECK(track_ref::copies == 0);
    CHECK(track_ref::moves == 0);

    multi(o, c, r);

    CHECK(track_copy::defaults == 1);
    CHECK(track_copy::copies == 4); // for each multicast + 1 for the msg call
    CHECK(track_copy::moves == 3); // for each caller
    CHECK(track_ref::defaults == 1);
    CHECK(track_ref::copies == 0);
    CHECK(track_ref::moves == 0);


    reset_tracks();
}

class a
{
public:
    int uni(track_copy cp, track_move&& mv, track_ref& ref)
    {
        auto x = std::move(mv);
        member = mv.val;
        return cp.val + mv.val + ref.val + x.val;
    }

    track_move uni_ret()
    {
        track_move r;
        r.val = 50;
        return r;
    }

    void multi(track_copy cp, track_ref& ref)
    {
        member = cp.val + ref.val;
    }

    int geta() const
    {
        return member;
    }

    int member = 80;
};

DYNAMIX_DEFINE_MIXIN(a, uni_msg & uni_ret_msg & multi_msg & geta_msg);

class b
{
public:
    void multi(track_copy cp, track_ref& ref)
    {
        member = cp.val + ref.val;
    }

    int getb() const
    {
        return member;
    }

    int member = 120;
};

DYNAMIX_DEFINE_MIXIN(b, multi_msg & getb_msg);

class c
{
public:
    void multi(track_copy cp, track_ref& ref)
    {
        member = cp.val + ref.val;
    }

    int getc() const
    {
        return member;
    }

    int member = 560;
};

DYNAMIX_DEFINE_MIXIN(c, multi_msg & getc_msg);

DYNAMIX_DEFINE_MESSAGE(uni);
DYNAMIX_DEFINE_MESSAGE(uni_ret);
DYNAMIX_DEFINE_MESSAGE(geta);
DYNAMIX_DEFINE_MESSAGE(getb);
DYNAMIX_DEFINE_MESSAGE(getc);
DYNAMIX_DEFINE_MESSAGE(multi);