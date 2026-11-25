// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/domain.hpp>
#include <dynamix/allocator.hpp>
#include <dynamix/mixin_info_data.hpp>
#include <dynamix/feature_info_data.hpp>

#include <itlib/span.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace dbench {

// a domain with Num_Mixins mixins defined without macros, each with 8 bytes of data
// every mixin has an internal and an external version
// the mixins can optionally implement features (each mixin implements different features)

// mixins with 8 bytes of data
template <int I>
struct mixin8 {
    uint64_t data = I;
};

struct bench_domain {
    static constexpr int Num_Mixins = 12;

    std::vector<std::unique_ptr<dynamix::util::feature_info_data>> features;
    std::vector<std::unique_ptr<dynamix::util::mixin_info_data>> internal;
    std::vector<std::unique_ptr<dynamix::util::mixin_info_data>> external;
    dynamix::domain dom; // declare after the infos, so it's destroyed before them

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

    explicit bench_domain(uint32_t features_per_mixin, dynamix::allocator alloc = {})
        : dom("bench", {}, 0, nullptr, alloc)
    {
        for (uint32_t i = 0; i < Num_Mixins * features_per_mixin; ++i) {
            auto& f = features.emplace_back(new dynamix::util::feature_info_data);
            dynamix::util::feature_info_data_builder fb(*f, "");
            fb.store_name("feature_" + std::to_string(i));
        }
        add_mixins(features_per_mixin, std::make_integer_sequence<int, Num_Mixins>{});
    }

    std::vector<const dynamix::mixin_info*> get_query(itlib::span<const uint32_t> indices, bool ext) const {
        std::vector<const dynamix::mixin_info*> ret;
        const auto& q = ext ? external : internal;
        for (auto i : indices) {
            ret.push_back(&q[i]->info);
        }
        return ret;
    }

    std::vector<const dynamix::mixin_info*> get_first_n(uint32_t n, uint32_t num_external = 0) const {
        std::vector<const dynamix::mixin_info*> ret;
        for (uint32_t i = 0; i < n; ++i) {
            const auto& q = i < n - num_external ? internal : external;
            ret.push_back(&q[i]->info);
        }
        return ret;
    }
};

} // namespace dbench
