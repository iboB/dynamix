// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//

#if defined(_TEST_MIXINS_HPP)
#   error("this file is not supposed to be included multiple times in the same module")
#endif

#define _TEST_MIXINS_HPP

DYNAMIX_DECLARE_MIXIN(a);
DYNAMIX_DECLARE_MIXIN(b);
DYNAMIX_DECLARE_MIXIN(c);

class a {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "a"; }
#endif
};
class b {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "b"; }
#endif
};
class c {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "c"; }
#endif
};

DYNAMIX_DEFINE_MIXIN(a, dynamix::none);
DYNAMIX_DEFINE_MIXIN(b, dynamix::none);
DYNAMIX_DEFINE_MIXIN(c, dynamix::none);
