// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "_throw_exception.hpp"
#include "domain.hpp"
#include "feature_info.hpp"
#include "mixin_info.hpp"
#include "type_mutation.hpp"
#include "exception.hpp"
#include "error_return.hpp"

#include <sstream>

namespace dynamix::priv {
namespace {
template <typename Exception>
class e {
    std::ostringstream out;
public:
    e(const domain& dom) {
        *this << dom << ": ";
    }

    e& operator<<(const domain& dom) {
        if (dom.name().empty()) {
            out << "unnamed domain";
        }
        else {
            out << dom.name();
        }
        return *this;
    }

    e& operator<<(const feature_info& info) {
        out << '\'' << info.name.to_std() << '\'';
        return *this;
    }

    e& operator<<(const mixin_info& info) {
        out << '\'' << info.name.to_std() << '\'';
        return *this;
    }

    e& operator<< (const mutation_rule_info& info) {
        out << '\'' << info.name.to_std() << '\'';
        return *this;
    }

    e& operator<<(const type_mutation& mut) {
        out << '{';
        bool first = true;
        for (const auto& m : mut.mixins) {
            if (first) {
                first = false;
            }
            else {
                out << ", ";
            }
            *this << *m;
        }
        out << '}';
        return *this;
    }

    template <typename T>
    e& operator<<(const T& t) {
        out << t;
        return *this;
    }

    [[noreturn]] ~e() noexcept(false) {
        throw Exception(out.str());
    }
};
}

void throw_id_registered(const domain& dom, const feature_info& info) {
    e<domain_error>(dom) << "register feature " << info << " with a valid id " << info.iid();
}

void throw_id_registered(const domain& dom, const mixin_info& info) {
    e<domain_error>(dom) << "register mixin " << info << " with a valid id " << info.iid();
}

void throw_empty_name(const domain& dom, const feature_info&) {
    e<domain_error>(dom) << "register feature with empty name";
}

void throw_empty_name(const domain& dom, const mixin_info&) {
    e<domain_error>(dom) << "register mixin with empty name";
}

void throw_empty_name(const domain& dom, const type_class&) {
    e<domain_error>(dom) << "register type class with empty name";
}

void throw_duplicate_name(const domain& dom, const feature_info& info) {
    e<domain_error>(dom) << "register feature with duplicate name '" << info.name.to_std() << '\'';
}

void throw_duplicate_name(const domain& dom, const mixin_info& info) {
    e<domain_error>(dom) << "register mixin with duplicate name '" << info.name.to_std() << '\'';
}

void throw_duplicate_name(const domain& dom, const type_class& tc) {
    e<domain_error>(dom) << "register type class with duplicate name '" << tc.name.to_std() << '\'';
}

void throw_info_has_domain(const domain& dom, const mixin_info& info) {
    e<domain_error>(dom) << "register mixin " << info << " which has a domain = " << *domain::from_c_handle(info.dom);;
}

void throw_unreg_foreign(const domain& dom, const feature_info& info) {
    e<domain_error>(dom) << "unregister foreign feature " << info << ", id = " << info.iid();
}
void throw_unreg_foreign(const domain& dom, const mixin_info& info) {
    e<domain_error> out(dom);
    out << "unregister foreign feature " << info << ", id = " << info.iid() << ", domain = ";
    if (info.dom) {
        out << *domain::from_c_handle(info.dom);
    }
    else {
        out << "<null>";
    }
}

void throw_no_func(const domain& dom, const mutation_rule_info& info) {
    e<domain_error>(dom) << "register mutation rule " << info << " with apply function = null";
}

void throw_no_func(const domain& dom, const type_class& tc) {
    e<domain_error>(dom) << "register type class '" << tc.name.to_std() << "' with match function = null";
}

void throw_mutation_rule_user_error(const domain& dom, const type_mutation& mut, const mutation_rule_info& info, error_return_t error) {
    e<mutation_error>(dom) << "applying mutation rule " << info << " to " << mut << " failed with error " << error;
}

}
