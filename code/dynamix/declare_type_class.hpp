// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "type_class.hpp"

#define DYNAMIX_DECLARE_TYPE_CLASS(tc) struct tc { static const dynamix::type_class m_dynamix_type_class; }
