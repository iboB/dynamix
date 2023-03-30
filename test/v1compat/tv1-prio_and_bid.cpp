// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/v1compat/core.hpp>
#include <dynamix/v1compat/next_bidder.hpp>

#include <sstream>

#include "doctest/doctest.h"

TEST_SUITE_BEGIN("v1 prio & bid");

using namespace dynamix::v1compat;

DYNAMIX_V1_DECLARE_MIXIN(a);
DYNAMIX_V1_DECLARE_MIXIN(b);
DYNAMIX_V1_DECLARE_MIXIN(c);
DYNAMIX_V1_DECLARE_MIXIN(d);

DYNAMIX_V1_MULTICAST_MESSAGE_1(void, trace, std::ostream&, out);
DYNAMIX_V1_MULTICAST_MESSAGE_1(void, priority_trace, std::ostream&, out);

DYNAMIX_V1_MESSAGE_1(void, bids_uni, std::ostream&, out);
DYNAMIX_V1_CONST_MESSAGE_1(void, bids_bad_uni, std::ostream&, out);
DYNAMIX_V1_MULTICAST_MESSAGE_1(void, bids_multi, std::ostream&, out);
DYNAMIX_V1_CONST_MULTICAST_MESSAGE_1(void, bids_multi_override, std::ostream&, out);

TEST_CASE("different_priority") {
    object o;

    mutate(o)
        .add<a>()
        .add<b>()
        .add<c>()
        .add<d>();

    CHECK(o.implements(priority_trace_msg));
    CHECK(o.type_info().num_implementers(priority_trace_msg) == 4);

    ostringstream sout;
    priority_trace(o, sout);
    CHECK(sout.str() == "210-1");
}

TEST_CASE("same_priority") {
    object o;

    mutate(o)
        .add<a>()
        .add<b>()
        .add<c>()
        .add<d>();

    CHECK(o.implements(trace_msg));
    CHECK(o.type_info().num_implementers(trace_msg) == 4);

    ostringstream sout;
    trace(o, sout);
    CHECK(sout.str() == "abcd");
}

TEST_CASE("bids") {
    object o;

    mutate(o)
        .add<a>()
        .add<b>()
        .add<c>()
        .add<d>();

    ostringstream sout;
    bids_uni(o, sout);
    CHECK(sout.str() == "abc");

#if DYNAMIX_V1_USE_EXCEPTIONS
    CHECK_THROWS_AS(bids_bad_uni(o, sout), bad_next_bidder_call);
#endif

    sout.str(string());
    bids_multi_override(o, sout);
    CHECK(sout.str() == "cd");

    sout.str(string());
    bids_multi(o, sout);
    CHECK(sout.str() == "bacd");

    mutate(o)
        .remove<c>()
        .remove<d>();

#if DYNAMIX_V1_USE_EXCEPTIONS
    CHECK_THROWS_AS(bids_uni(o, sout), bad_next_bidder_call);
#endif

    sout.str(string());
    bids_multi_override(o, sout);
    CHECK(sout.str() == "ab");

    sout.str(string());
    bids_multi(o, sout);
    CHECK(sout.str() == "ba");
}

// test for issue #20
class parent {
public:
    void priority_trace(std::ostream& out) {
        out << "-1";
    }
};

class a : public parent {
public:
    void trace(std::ostream& out) {
        out << "a";
    }

    void bids_uni(std::ostream& out) {
        out << "a";
        CHECK(DYNAMIX_V1_HAS_NEXT_BIDDER(bids_uni_msg));
        DYNAMIX_V1_CALL_NEXT_BIDDER(bids_uni_msg, out);
    }

    void bids_bad_uni(std::ostream& out) const {
        out << "a";
        CHECK(!DYNAMIX_V1_HAS_NEXT_BIDDER(bids_bad_uni_msg));
        DYNAMIX_V1_CALL_NEXT_BIDDER(bids_bad_uni_msg, out);
    }

    void bids_multi(std::ostream& out) {
        out << "a";
    }

    void bids_multi_override(std::ostream& out) const {
        out << "a";
    }
};

class b {
public:
    void trace(std::ostream& out) {
        out << "b";
    }

    void priority_trace(std::ostream& out) {
        out << "2";
    }

    void bids_uni(std::ostream& out) {
        out << "b";
        DYNAMIX_V1_CALL_NEXT_BIDDER(bids_uni_msg, out);
    }

    void bids_bad_uni(std::ostream& out) const {
        out << "b";
    }

    void bids_multi(std::ostream& out) {
        out << "b";
        CHECK(DYNAMIX_V1_HAS_NEXT_BIDDER(bids_multi_msg));
        DYNAMIX_V1_CALL_NEXT_BIDDER(bids_multi_msg, out);
    }

    void bids_multi_override(std::ostream& out) const {
        out << "b";
    }
};

class c {
public:
    void trace(std::ostream& out) {
        out << "c";
    }

    void priority_trace(std::ostream& out) {
        out << "1";
    }

    void bids_uni(std::ostream& out) {
        out << "c";
        CHECK(!DYNAMIX_V1_HAS_NEXT_BIDDER(bids_uni_msg));
    }

    void bids_multi(std::ostream& out) {
        out << "c";
    }

    void bids_multi_override(std::ostream& out) const {
        out << "c";
    }
};

class d {
public:
    void trace(std::ostream& out) {
        out << "d";
    }

    void priority_trace(std::ostream& out) {
        out << "0";
    }

    void bids_uni(std::ostream& out) {
        out << "d";
    }

    void bids_multi(std::ostream& out) {
        out << "d";
    }

    void bids_multi_override(std::ostream& out) const {
        out << "d";
    }
};

// this order should be important if the messages aren't sorted by mixin name
DYNAMIX_V1_DEFINE_MIXIN(b,
    trace_msg& priority(2, priority_trace_msg)
    & priority(1, bid(1, bids_uni_msg))& priority(-1, bids_bad_uni_msg)& bids_multi_override_msg& bid(1, bids_multi_msg));
DYNAMIX_V1_DEFINE_MIXIN(a,
    trace_msg& priority(-1, priority_trace_msg)
    & bid(2, priority(1, bids_uni_msg))& bids_bad_uni_msg& bids_multi_override_msg& bids_multi_msg);
DYNAMIX_V1_DEFINE_MIXIN(c,
    trace_msg& priority(1, priority_trace_msg)
    & priority(1, bids_uni_msg)& bid(1, bids_multi_override_msg)& bids_multi_msg);
DYNAMIX_V1_DEFINE_MIXIN(d,
    trace_msg& priority_trace_msg& bids_uni_msg& bid(1, bids_multi_override_msg)& bid(1, bids_multi_msg));

DYNAMIX_V1_DEFINE_MESSAGE(trace);
DYNAMIX_V1_DEFINE_MESSAGE(priority_trace);
DYNAMIX_V1_DEFINE_MESSAGE(bids_uni);
DYNAMIX_V1_DEFINE_MESSAGE(bids_bad_uni);
DYNAMIX_V1_DEFINE_MESSAGE(bids_multi);
DYNAMIX_V1_DEFINE_MESSAGE(bids_multi_override);
