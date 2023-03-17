// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "error_return.hpp"
#include <stdexcept>

namespace dynamix {
struct DYNAMIX_API exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    ~exception();
};

struct DYNAMIX_API domain_error : public exception {
    using exception::exception;
    ~domain_error();
};

struct DYNAMIX_API domain_user_error : public domain_error {
    domain_user_error(const char* text, error_return_t e);
    error_return_t error;
};

struct DYNAMIX_API mutation_error : public exception {
    using exception::exception;
    ~mutation_error();
};

struct DYNAMIX_API mutation_user_error : public mutation_error {
    mutation_user_error(const char* text, error_return_t e);
    error_return_t error;
};

struct DYNAMIX_API compare_error : public exception {
    using exception::exception;
    ~compare_error();
};

struct DYNAMIX_API bad_feature_access : public exception {
    using exception::exception;
    ~bad_feature_access();
};

}
