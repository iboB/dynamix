// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/type_class.h"
#include "type_class_fwd.hpp"

namespace dynamix {
using type_class_id = dnmx_type_class_id;
inline constexpr type_class_id invalid_type_class_id = type_class_id{dnmx_invalid_id};
}
