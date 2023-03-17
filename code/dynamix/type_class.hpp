// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"

namespace dynamix {
class type;

class DYNAMIX_API type_class {
public:
    virtual ~type_class();
    virtual bool matches(const type& type) const noexcept = 0;
};

}
