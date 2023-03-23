// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "common_feature_info.hpp"
#include "any.hpp"

#include "compat/pmr/string.hpp"

namespace dynamix::util {

// a type which contains a feature_info with optional storage for fields
struct feature_info_data {
    explicit feature_info_data(const allocator& alloc = {}) noexcept
        : stored_name(alloc)
    {}

    allocator get_allocator() const noexcept { return stored_name.get_allocator(); }

    common_feature_info info;

    compat::pmr::string stored_name;
    any default_payload_storage;
};

// EDSL which builds feature_info_data:
class feature_info_data_builder {
    feature_info_data& m_data;
public:
    feature_info_data_builder(feature_info_data& data, dnmx_sv name) noexcept
        : m_data(data)
    {
        m_data.info.name = name;
    }

    feature_info_data_builder(feature_info_data& d, std::string_view name) noexcept
        : feature_info_data_builder(d, dnmx_sv::from_std(name))
    {}

    // adopting ctor: doesn't try to initialize data
    feature_info_data_builder(std::nullptr_t, feature_info_data& data) noexcept
        : m_data(data)
    {}

    feature_info_data& get_data() const noexcept { return m_data; }

    using self = feature_info_data_builder;

    // override name from view
    self& name(std::string_view name) noexcept {
        m_data.info.name = dnmx_sv::from_std(name);
        return *this;
    }

    // override and store store name
    self& store_name(std::string_view sname) {
        m_data.stored_name = std::string(sname);
        return name(m_data.stored_name);
    }

    self& default_payload(any storage) {
        m_data.default_payload_storage = storage;
        m_data.info.default_payload = m_data.default_payload_storage.get();
        return *this;
    }

    template <typename Payload>
    self& default_payload_by(Payload& pl) {
        return default_payload(fwd_any(pl));
    }

    template <typename Payload>
    self& default_payload_with(Payload&& pl) {
        return default_payload(make_any(m_data.get_allocator(), std::forward<Payload>(pl)));
    }
};

}