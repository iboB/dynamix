// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/object.hpp>

#include <cstdint>
#include <random>

dynamix::object make_obj_accumulator(std::minstd_rand& rnd);

uint64_t obj_accumulate(const dynamix::object& obj, uint32_t val);
