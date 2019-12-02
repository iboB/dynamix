// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"

// this file is used to export inline-only classes for msvc
// gcc and clang deal with those fine but msvc needs some compilation unit
// to explicitly export their symbols

#include "dynamix/internal/feature_parser.hpp" // for feature_parser_phase_1
