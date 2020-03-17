// DynaMix
// Copyright (c) 2013-2020 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include "dynamix/define_type_class.hpp"
#include "dynamix/domain.hpp"

namespace dynamix
{

type_class::type_class(define_type_class builder)
    : _match_func(builder.func)
{
    if (builder.do_register)
    {
        internal::domain::safe_instance().register_type_class(*this);
    }
}

type_class::~type_class() = default;

}
