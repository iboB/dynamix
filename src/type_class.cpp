// DynaMix
// Copyright (c) 2013-2020 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include "dynamix/type_class.hpp"
#include "dynamix/domain.hpp"

namespace dynamix
{

type_class::type_class(type_class::match_func func, bool register_globally /*= false*/)
    : _match_func(func)
{
    if (register_globally)
    {
        internal::domain::safe_instance().register_type_class(*this);
    }
}

type_class::~type_class()
{
    if (is_registered())
    {
        internal::domain::safe_instance().unregister_type_class(*this);
    }
}

}
