// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "api.h"
#include "domain_tag.hpp"

#define DYNAMIX_EXPORT TESTLIB_API
using DYNAMIX_DOMAIN_TAG = test_tag;

#include <dynamix/implement.inl>

namespace dynamix {
dynamix::domain_allocator init_domain_allocator(test_tag*) { return {}; }
}
