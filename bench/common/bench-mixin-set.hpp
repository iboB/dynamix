// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/domain.hpp>
#include <dynamix/allocator.hpp>
#include <dynamix/mixin_info_data.hpp>
#include <dynamix/feature_info_data.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace dbench {

// a domain with NUM_MIXINS mixins defined without macros, each with 8 bytes of data
// every mixin has an internal and an external version
// the mixins can optionally implement features (each mixin implements different features)

// mixins with 8 bytes of data
template <int I>
struct mixin8 {
    uint64_t data = I;
};

constexpr int NUM_MIXINS = 12;

struct mixin_set {
    std::vector<std::unique_ptr<dynamix::util::feature_info_data>> features;
    std::vector<std::unique_ptr<dynamix::util::mixin_info_data>> internal;
    std::vector<std::unique_ptr<dynamix::util::mixin_info_data>> external;
    dynamix::domain dom; // declared after the infos, so it's destroyed before them

    template <int I>
    void add_mixin(uint32_t features_per_mixin) {
        auto add = [&](std::vector<std::unique_ptr<dynamix::util::mixin_info_data>>& vec, bool ext) {
            auto& data = vec.emplace_back(new dynamix::util::mixin_info_data);
            dynamix::util::mixin_info_data_builder<mixin8<I>> b(*data, "");
            b.store_name((ext ? "ext_" : "int_") + std::to_string(I));
            b.force_external(ext);
            for (uint32_t i = 0; i < features_per_mixin; ++i) {
                // each mixin implements different features
                auto& f = *features[I * features_per_mixin + i];
                b.implements_with(f.info, 0);
            }
            data->register_in(dom);
        };
        add(internal, false);
        add(external, true);
    }

    template <int... I>
    void add_mixins(uint32_t features_per_mixin, std::integer_sequence<int, I...>) {
        (add_mixin<I>(features_per_mixin), ...);
    }

    explicit mixin_set(uint32_t features_per_mixin, dynamix::allocator alloc = {})
        : dom("bench", {}, 0, nullptr, alloc)
    {
        for (uint32_t i = 0; i < NUM_MIXINS * features_per_mixin; ++i) {
            auto& f = features.emplace_back(new dynamix::util::feature_info_data);
            dynamix::util::feature_info_data_builder fb(*f, "");
            fb.store_name("feature_" + std::to_string(i));
        }
        add_mixins(features_per_mixin, std::make_integer_sequence<int, NUM_MIXINS>{});
    }

    std::vector<const dynamix::mixin_info*> query(const std::vector<uint32_t>& indices, bool ext) const {
        std::vector<const dynamix::mixin_info*> ret;
        for (auto i : indices) {
            ret.push_back(&(ext ? external : internal)[i]->info);
        }
        return ret;
    }

    std::vector<const dynamix::mixin_info*> first_n(uint32_t n, uint32_t num_external = 0) const {
        std::vector<const dynamix::mixin_info*> ret;
        for (uint32_t i = 0; i < n; ++i) {
            ret.push_back(&(i < n - num_external ? internal : external)[i]->info);
        }
        return ret;
    }
};

} // namespace dbench
