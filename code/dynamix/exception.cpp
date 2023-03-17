// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "exception.hpp"

namespace dynamix {
// exporting the vtable
exception::~exception() = default;

domain_error::~domain_error() = default;

domain_user_error::domain_user_error(const char* text, error_return_t e)
    : domain_error(text)
    , error(e)
{}

mutation_error::~mutation_error() = default;

mutation_user_error::mutation_user_error(const char* text, error_return_t e)
    : mutation_error(text)
    , error(e)
{}

compare_error::~compare_error() = default;

bad_feature_access::~bad_feature_access() = default;
}
