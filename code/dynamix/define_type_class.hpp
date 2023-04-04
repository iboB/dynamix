// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "declare_type_class.hpp"
#include "type.hpp"

// define a type class with bool(const type&);
#define DYNAMIX_DEFINE_TYPE_CLASS_WITH(tc, func) \
    const dynamix::type_class tc::m_dynamix_type_class = {dnmx_make_sv_lit(#tc), func, nullptr, dnmx_invalid_type_class_id};
