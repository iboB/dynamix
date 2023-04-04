// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/bits/pp.h"
#include "type_class.hpp"

#define DYNAMIX_DECLARE_EXPORTED_TYPE_CLASS(export, tc) struct export tc { static const dynamix::type_class m_dynamix_type_class; }
#define DYNAMIX_DECLARE_TYPE_CLASS(tc) DYNAMIX_DECLARE_EXPORTED_TYPE_CLASS(I_DNMX_PP_EMPTY(), tc)
