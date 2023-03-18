// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "object_mutate_ops.hpp"
namespace dynamix {
// export vtable
bool object_mutate_op::overrides_init() const noexcept { return true; }
}
