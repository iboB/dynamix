// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "object_of.hpp"

// utility macro to get the object of a mixin from the point of view of the mixin class
// be careful to only call from mixins (UB if not called from a mixin)
#define dm_this ::dynamix::object_of(this)
