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

struct DYNAMIX_API type_error : public exception {
    using exception::exception;
    ~type_error();
};

struct DYNAMIX_API object_error : public exception {
    using exception::exception;
    ~object_error();
};

struct DYNAMIX_API bad_feature_access : public exception {
    using exception::exception;
    ~bad_feature_access();
};

}
