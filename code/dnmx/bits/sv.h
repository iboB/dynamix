// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "empty_init.h"

#include <stdbool.h>
#include <string.h>

#if defined(__cplusplus)
#include <string_view>
#endif

// string view

typedef struct dnmx_sv
{
    const char* begin; // start of string
    const char* end; // one after the last character of the string

#if defined(__cplusplus)
    std::string_view to_std() const noexcept {
        return std::string_view(begin, end - begin);
    }
    static dnmx_sv from_std(const std::string_view& sv) noexcept {
        dnmx_sv ret;
        ret.begin = sv.data();
        ret.end = ret.begin + sv.length();
        return ret;
    }
    bool operator==(const std::string_view& sv) const noexcept {
        return to_std() == sv;
    }
    friend bool operator==(const std::string_view& sv, const dnmx_sv& d) noexcept {
        return sv == d.to_std();
    }
    bool operator==(const dnmx_sv& other) const noexcept {
        return to_std() == other.to_std();
    }
    bool operator!=(const std::string_view& sv) const noexcept {
        return to_std() != sv;
    }
    friend bool operator!=(const std::string_view& sv, const dnmx_sv& d) noexcept {
        return sv != d.to_std();
    }
    bool operator!=(const dnmx_sv& other) const noexcept {
        return to_std() != other.to_std();
    }
    bool operator<(const dnmx_sv& other) const noexcept {
        return to_std() < other.to_std();
    }
    bool empty() const noexcept {
        return begin == end;
    }
#endif
} dnmx_sv;

#if defined(__cplusplus)
extern "C" {
#endif

static inline dnmx_sv dnmx_make_sv_be(const char* begin, const char* end) {
    dnmx_sv ret = {begin, end};
    return ret;
}

static inline dnmx_sv dnmx_make_sv_len(const char* begin, size_t length) {
    return dnmx_make_sv_be(begin, begin + length);
}

static inline dnmx_sv dnmx_make_sv_str(const char* str) {
    if (!str) return DNMX_EMPTY_T(dnmx_sv);
    return dnmx_make_sv_be(str, str + strlen(str));
}

#define dnmx_make_sv_lit(lit) dnmx_make_sv_len(lit, sizeof(lit)-1)

static inline bool dnmx_sv_is_null(dnmx_sv sv) { return !sv.begin; }
static inline bool dnmx_sv_is_empty(dnmx_sv sv) { return sv.begin == sv.end; }
static inline size_t dnmx_sv_len(dnmx_sv sv) { return sv.end - sv.begin; }

static inline int dnmx_sv_cmp(dnmx_sv a, dnmx_sv b) {
    // avoid memcmp with null
    if (a.begin == b.begin) return 0;
    size_t alen = dnmx_sv_len(a);
    if (!b.begin) return !!alen;
    size_t blen = dnmx_sv_len(b);
    if (!a.begin) return -!!blen;

    if (alen == blen) {
        return memcmp(a.begin, b.begin, alen);
    }
    else if (alen < blen) {
        int cmp = memcmp(a.begin, b.begin, alen);
        return cmp == 0 ? -1 : cmp;
    }
    else {
        int cmp = memcmp(a.begin, b.begin, blen);
        return cmp == 0 ? 1 : cmp;
    }
}

static inline bool dnmx_sv_eq(dnmx_sv a, dnmx_sv b) {
    return dnmx_sv_cmp(a, b) == 0;
}

static inline bool dnmx_sv_starts_with(dnmx_sv sv, const char* prefix) {
    size_t svlen = dnmx_sv_len(sv);
    size_t plen = strlen(prefix);
    if (plen > svlen) return false; // prefix longer than string
    if (svlen == 0 && plen == 0) return true; // avoid memcmp with null
    return memcmp(sv.begin, prefix, plen) == 0;
}

static inline const char* dnmx_sv_find_first(dnmx_sv sv, char q) {
    size_t len = dnmx_sv_len(sv);
    if (!len) return NULL; // avoid memchr with null
    return (const char*)memchr(sv.begin, q, len);
}

static inline const char* dnmx_sv_find_last(dnmx_sv sv, char q) {
    size_t len = dnmx_sv_len(sv);
    while (len--) {
        if (sv.begin[len] == q) return sv.begin + len;
    }
    return NULL;

    // future:
    // if (!len) return NULL; // avoid memrchr with null
    // return (const char*)memrchr(sv.begin, q, len);
}

#if defined(__cplusplus)
}
#endif
