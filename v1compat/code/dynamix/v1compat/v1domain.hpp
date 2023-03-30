// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <splat/symbol_export.h>
#include <dynamix/declare_domain.hpp>

#if DYNAMIX_V1COMPAT_SHARED
#   if BUILDING_DYNAMIX_V1COMPAT
#       define DYNAMIX_V1COMPAT_API SYMBOL_EXPORT
#   else
#       define DYNAMIX_V1COMPAT_API SYMBOL_IMPORT
#   endif
#else
#   define DYNAMIX_V1COMPAT_API
#endif


namespace dynamix::v1compat {
struct domain_tag;
DYNAMIX_DECLARE_EXPORTED_DOMAIN(DYNAMIX_V1COMPAT_API, domain_tag);
}
