// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "bmm-msg.hpp"
#include "bmm-macros.hpp"

#include <dynamix/declare_domain.hpp>
#include <dynamix/define_domain.hpp>
#include <dynamix/declare_mixin.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/declare_msg.hpp>
#include <dynamix/msg/define_msg.hpp>

struct bench;
DYNAMIX_DECLARE_DOMAIN(bench);

DYNAMIX_DECLARE_MSG(accumulate_msg, accumulate, void, (const dynamix::object&, uint64_t&, uint32_t));
