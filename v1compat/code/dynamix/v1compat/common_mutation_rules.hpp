// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "mutation_rule.hpp"
#include <dynamix/globals.hpp>
#include <dynamix/type_mutation.hpp>
#include <itlib/qalgorithm.hpp>
#include <vector>

namespace dynamix::v1compat {
namespace impl {
class DYNAMIX_V1COMPAT_API mixin_list {
protected:
    std::vector<const mixin_info*> infos;
public:
    void add(const mixin_info& info) { infos.push_back(&info); }
    void add(std::string_view name);
    template <typename Mixin>
    void add() { add(g::get_mixin_info<Mixin>()); }
};
}

class dependent_mixins : public mutation_rule, public impl::mixin_list {
    const mixin_info* master = nullptr;
public:
    template <typename Mixin>
    void set_master() {
        master = &g::get_mixin_info<Mixin>();
    }

    virtual void apply_to(type_mutation& mut) override {
        if (mut.has(*master)) {
            for (auto i : infos) {
                mut.add_if_lacking(*i);
            }
        }
        else {
            for (auto i : infos) {
                mut.remove(*i);
            }
        }
    }
};

class mutually_exclusive_mixins : public mutation_rule, public impl::mixin_list {
public:
    virtual void apply_to(type_mutation& mut) override {
        // we want to keep the last
        // so reverse, keep the first, then reverse again
        std::reverse(mut.mixins.begin(), mut.mixins.end());
        bool found = false;
        itlib::erase_all_if(mut.mixins, [&](const mixin_info* i) {
            auto f = itlib::pfind(infos, i);
            if (!f) return false; // don't care about this one
            if (found) return true; // erase if another mutually exclusive was found
            found = true;
            return false; // keep the first
        });
        std::reverse(mut.mixins.begin(), mut.mixins.end());
    }
};

template <typename Mixin>
class deprecated_mixin : public mutation_rule {
public:
    virtual void apply_to(type_mutation& mut) override {
        mut.remove<Mixin>();
    }
};

template <typename Mixin>
class mandatory_mixin : public mutation_rule {
public:
    virtual void apply_to(type_mutation& mut) override {
        mut.add_if_lacking<Mixin>();
    }
};

template <typename SourceMixin, typename TargetMixin>
class substitute_mixin : public mutation_rule {
    virtual void apply_to(type_mutation& mut) override {
        if (mut.has<SourceMixin>()) {
            mut.remove<SourceMixin>();
            mut.add<TargetMixin>();
        }
    }
};

}
