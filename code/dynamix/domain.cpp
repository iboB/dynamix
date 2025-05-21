// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "domain.hpp"
#include "mixin_info.hpp"
#include "type.hpp"
#include "type_mutation.hpp"
#include "object_mixin_data.hpp"
#include "alloc_util.hpp"
#include "feature_info.hpp"
#include "feature_for_mixin.hpp"
#include "mutation_rule_info.hpp"
#include "throw_exception.hpp"
#include "domain_traverse.hpp"

#include <itlib/qalgorithm.hpp>
#include <itlib/data_mutex.hpp>
#include <itlib/flat_map.hpp>

#include "compat/pmr/vector.hpp"
#include "compat/pmr/set.hpp"
#include "compat/pmr/map.hpp"

#include <memory>
#include <limits>
#include <cassert>

#if 0
#include <mutex>
namespace dynamix {
// fake mutex which can be used in tsan scenarios
// for some reason recursive locks on std::shared_mutex don't trigger tsan errors
class shared_mutex {
    std::mutex m_mutex;
public:
    void lock_shared() { m_mutex.lock(); }
    void unlock_shared() { m_mutex.unlock(); }
    bool try_lock_shared() { return m_mutex.try_lock(); }
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    bool try_lock() { return m_mutex.try_lock(); }
};
}
#else
#include <shared_mutex>
namespace dynamix {
using shared_mutex = std::shared_mutex;
}
#endif

namespace dynamix {

namespace {
error_return_t sort_by_canonical_order(dnmx_type_mutation_handle mutation, uintptr_t) {
    auto& mixins = type_mutation::from_c_handle(mutation)->mixins;
    std::sort(mixins.begin(), mixins.end(), canonical_mixin_order{});
    return result_success;
}

using mixin_info_span = const itlib::span<const mixin_info* const>;

template <typename T>
using data_mutex = itlib::data_mutex<T, shared_mutex>;

bool mixin_span_less(const mixin_info_span& a, const mixin_info_span& b) {
    // first compare sizes, which greatly improves performance of searches
    // (we don't actually care about the order of the types or queries)
    size_t as = a.size(), bs = b.size();
    if (as != bs) return as < bs;
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}
//bool mixin_span_equal(const mixin_info_span& a, const mixin_info_span& b) {
//    return std::equal(a.begin(), a.end(), b.begin(), b.end());
//}
}

class domain::impl {
public:
    domain& m_domain;

    allocator m_allocator;

    // registry of type elements
    // this is separate from the type registry because it can be locked independently
    // (and does get locked recursively when applying mutation rules)
    struct element_registry {
        element_registry(allocator alloc)
            : sparse_features(alloc)
            , sparse_mixins(alloc)
            , sparse_type_classes(alloc)
        {}

        // sparse arrays of info per id
        // null elements have been unregistered and are free slots for future registers
        compat::pmr::vector<const feature_info*> sparse_features;
        compat::pmr::vector<const mixin_info*> sparse_mixins;
        compat::pmr::vector<const type_class*> sparse_type_classes;
    };
    data_mutex<element_registry> m_element_registry;

    struct deleter {
        void operator()(const type* ptr) {
            const void* cvptr = ptr;
            auto vptr = const_cast<void*>(cvptr);
            ptr->dom.m_impl->m_allocator.deallocate_bytes(vptr, ptr->buf_size, alignof(type));
        }
    };

    template <typename T>
    using uptr = std::unique_ptr<T, deleter>;

    struct rule_compare {
        bool operator()(const mutation_rule_info* a, const mutation_rule_info* b) const {
            if (a->order_priority != b->order_priority) return a->order_priority < b->order_priority;
            auto name_cmp = dnmx_sv_cmp(a->name, b->name);
            if (name_cmp != 0) return name_cmp < 0;
            return a < b;
        }
    };
    using mutation_rule_map = itlib::flat_map<const mutation_rule_info*, uint32_t, rule_compare, compat::pmr::vector<std::pair<const mutation_rule_info*, uint32_t>>>;

    struct type_compare {
        bool operator()(const uptr<const type>& a, const uptr<const type>& b) const {
            return mixin_span_less(a->mixins, b->mixins);
        }
        bool operator()(const uptr<const type>& a, mixin_info_span& b) const {
            return mixin_span_less(a->mixins, b);
        }
        bool operator()(mixin_info_span& a, const uptr<const type>& b) const {
            return mixin_span_less(a, b->mixins);
        }
        using is_transparent = void;
    };
    using type_set = compat::pmr::set<uptr<const type>, type_compare>;

    using type_query = compat::pmr::vector<const mixin_info*>;
    struct type_query_compare {
        bool operator()(const type_query& a, const type_query& b) const {
            return mixin_span_less(a, b);
        }
        bool operator()(const type_query& a, mixin_info_span& b) const {
            return mixin_span_less(a, b);
        }
        bool operator()(mixin_info_span& a, const type_query& b) const {
            return mixin_span_less(a, b);
        }
        using is_transparent = void;
    };
    using type_query_map = compat::pmr::map<type_query, const type*, type_query_compare>;

    // registry of types and helpers
    // it independently locked from the element registry
    struct type_registry {
        type_registry(allocator alloc)
            : mutation_rules({}, alloc)
            , types(alloc)
            , type_queries(alloc)
        {}

        // sorted rules with their refcounts
        mutation_rule_map mutation_rules;

        // existing types
        type_set types;

        // stored type queries
        // with them we avoid applying mutation rules for the same type query
        type_query_map type_queries;
    };
    data_mutex<type_registry> m_type_registry;

    type m_empty_type;

    const mutation_rule_info m_canonicalize_rule = {
        dnmx_make_sv_lit("canonicalize types"),
        sort_by_canonical_order,
        0,
        std::numeric_limits<int32_t>::max()
    };

    impl(domain& domain, allocator alloc)
        : m_domain(domain)
        , m_allocator(std::move(alloc))
        , m_element_registry(m_allocator)
        , m_type_registry(m_allocator)
        , m_empty_type(domain, 0)
    {
        if (m_domain.m_settings.canonicalize_types) {
            // we solve this requirement by adding a mutation rule which sorts the mixins of the new type
            add_mutation_rule(m_canonicalize_rule);
        }
    }

    template <typename T>
    void basic_register_l(T& info, compat::pmr::vector<const T*>& sparse, bool enforce_unique_names) {
        using id_t = decltype(info.id);
        constexpr id_t invalid = id_t{dnmx_invalid_id};
        if (info.id != invalid) throw_exception::id_registered(m_domain, info);
        if (enforce_unique_names && info.name.empty()) throw_exception::empty_name(m_domain, info);

        id_t free_id = invalid;

        if (enforce_unique_names) {
            // search in reverse order while also checking for name clashes
            // after this loop, the least free_id will be saved
            for (dnmx_id_int_t i = dnmx_id_int_t(sparse.size()); i-- > 0; ) {
                auto reg = sparse[i];
                if (reg) {
                    assert(reg->iid() == i); // sanity check
                    if (reg->name == info.name) throw_exception::duplicate_name(m_domain, info);
                }
                else {
                    free_id = id_t{i};
                }
            }
        }
        else {
            // simply find first free id
            for (dnmx_id_int_t i = 0; i < sparse.size(); ++i) {
                if (!sparse[i]) {
                    free_id = id_t{i};
                    break;
                }
            }
        }

        if (free_id == invalid) {
            free_id = id_t{dnmx_id_int_t(sparse.size())};
            sparse.push_back(nullptr);
        }

        info.id = free_id;
        sparse[info.iid()] = &info;
    }

    template <typename T>
    void basic_unregister_l(T& info, compat::pmr::vector<const T*>& sparse) {
        // info is not our own?
        if (info.iid() >= sparse.size() || sparse[info.iid()] != &info) throw_exception::unreg_foreign(m_domain, info);

        sparse[info.iid()] = nullptr; // free slot
        info.id = decltype(info.id){dnmx_invalid_id}; // invalidate info
    }

    void register_feature(feature_info& info) {
        basic_register_l(info, m_element_registry.unique_lock()->sparse_features, !m_domain.m_settings.allow_duplicate_feature_names);
    }

    // to be pedantic when we clear features we should clear all object types which
    // implement them, but this seems to be unnecessary
    // if a feature has found its way into a type info, then there must be a mixin that uses it
    // if the feature is local to a module, then that mixin must be local to the module too
    // since the feature is being unregistered because the module is unloaded,
    // then surely the mixin should be unregistered too
    // then all types infos containing that mixin (and in turn this feature)
    // will be dropped eventually
    // for a short while we will live with incosistent lists of feature and mixins
    // if something breaks because of this inconsitency, then this would have been a
    // break anyway - the mixins referencing this feature would keep on living??
    void unregister_feature(feature_info& info) {
        basic_unregister_l(info, m_element_registry.unique_lock()->sparse_features);
    }

    void register_mixin(mixin_info& info) {
        if (info.dom != nullptr && info.dom != &m_domain) throw_exception::info_has_domain(m_domain, info);
        auto reg = m_element_registry.unique_lock();

        // register mixin's features
        for (auto& f : info.features_span()) {
            if (f.info->id != invalid_feature_id) {
                // already registered
                assert(reg->sparse_features[f.info->iid()] == f.info); // sanity
                continue;
            }
            // we need to const_cast. this is the price of lazy ops in C++
            basic_register_l(*const_cast<feature_info*>(f.info), reg->sparse_features, !m_domain.m_settings.allow_duplicate_feature_names);

            // note that if the registration fails, previously registered features are not rolled back
            // and they shouldnt: features are separate from mixins
        }

        // register mixin itself
        basic_register_l(info, reg->sparse_mixins, !m_domain.m_settings.allow_duplicate_mixin_names);
        info.dom = &m_domain;
    }

    void unregister_mixin(mixin_info& info) {
        {
            auto treg = m_type_registry.unique_lock();
            // since this mixin is no longer valid
            // remove all queries which contain it either as key or as value
            auto& queries = treg->type_queries;
            for (auto iq = queries.begin(); iq != queries.end(); ) {
                if (iq->second->has(info.id)) {
                    // query leads to a type which contains mixin
                    iq = queries.erase(iq);
                }
                else if (itlib::pfind(iq->first, &info)) {
                    // query references mixin
                    iq = queries.erase(iq);
                }
                else {
                    ++iq;
                }
            }

            // ... and remove all types which reference it
            auto& types = treg->types;
            for (auto it = types.begin(); it != types.end(); ) {
                auto& t = *it;
                if (!t->has(info.id)) {
                    // type does't have mixin
                    ++it;
                    continue;
                }

                // removing a type with active objects?
                // UB and crashes await
                assert(t->num_objects() == 0);

                it = types.erase(it);
            }
        }

        auto reg = m_element_registry.unique_lock();
        auto& sparse_mixins = reg->sparse_mixins;

        // mixin is not our own?
        if (info.iid() >= sparse_mixins.size() || sparse_mixins[info.iid()] != &info) throw_exception::unreg_foreign(m_domain, info);
        sparse_mixins[info.iid()] = nullptr; // free slot

        // invalidate info
        info.id = invalid_mixin_id;
        info.dom = nullptr;
    }

    void register_type_class(const type_class& new_tc) {
        if (new_tc.name.empty()) throw_exception::empty_name(m_domain, new_tc);
        if (!new_tc.matches) throw_exception::no_func(m_domain, new_tc);

        auto reg = m_element_registry.unique_lock();

        // search in reverse order while also checking for name clashes
        const type_class** free_slot = nullptr;
        for (auto i = reg->sparse_type_classes.rbegin(); i != reg->sparse_type_classes.rend(); ++i) {
            auto tc = *i;
            if (tc) {
                if (tc->name == new_tc.name) throw_exception::duplicate_name(m_domain, new_tc);
            }
            else {
                free_slot = &tc;
            }
        }

        if (!free_slot) {
            free_slot = &reg->sparse_type_classes.emplace_back();
        }

        *free_slot = &new_tc;
    }

    void unregister_type_class(const type_class& tc) {
        auto reg = m_element_registry.unique_lock();

        for (auto& rtc : reg->sparse_type_classes) {
            if (rtc == &tc) {
                rtc = nullptr;
                return;
            }
        }
    }

    template <typename Id, typename T>
    static const T* basic_get_by_id_l(Id id, const compat::pmr::vector<const T*>& sparse) noexcept {
        if (id.i >= sparse.size()) return nullptr;
        return sparse[id.i];
    }

    template <typename T>
    static const T* basic_get_by_name_l(std::string_view name, const compat::pmr::vector<const T*>& sparse) noexcept {
        for (auto& info : sparse) {
            if (info && info->name == name) return info;
        }
        return nullptr;
    }

    const mixin_info* get_mixin_info(mixin_id id) noexcept {
        return basic_get_by_id_l(id, m_element_registry.shared_lock()->sparse_mixins);
    }
    const mixin_info* get_mixin_info(std::string_view name) noexcept {
        return basic_get_by_name_l(name, m_element_registry.shared_lock()->sparse_mixins);
    }

    const feature_info* get_feature_info(feature_id id) noexcept {
        return basic_get_by_id_l(id, m_element_registry.shared_lock()->sparse_features);
    }
    const feature_info* get_feature_info(std::string_view name) noexcept {
        return basic_get_by_name_l(name, m_element_registry.shared_lock()->sparse_features);
    }

    const type_class* get_type_class(std::string_view name) noexcept {
        return basic_get_by_name_l(name, m_element_registry.shared_lock()->sparse_type_classes);
    }

    void add_mutation_rule(const mutation_rule_info& info) {
        if (!info.apply) throw_exception::no_func(m_domain, info);

        auto reg = m_type_registry.unique_lock();

        auto& rc = reg->mutation_rules[&info];
        ++rc;
        if (rc != 1) return; // not first-time registering

        // first time registered
        // we need to invalidate stored type queries
        // we can't tell which ones the rule affects, so we have to invalidate them all
        reg->type_queries.clear();
    }
    void remove_mutation_rule(const mutation_rule_info& info) noexcept {
        auto reg = m_type_registry.unique_lock();
        auto f = reg->mutation_rules.find(&info);
        if (f == reg->mutation_rules.end()) return; // not added
        assert(f->second > 0);
        --f->second;
        if (f->second > 0) return; // more refs exist

        // refs are zero, so remove rule and invalidate stored type queries
        reg->mutation_rules.erase(f);
        // we can't tell which ones the rule affects, so we have to invalidate them all
        reg->type_queries.clear();
    }

    // applies mutation rules for mutation and returns the original query to be preserved
    // this is also an optimization opportunity
    // if the mutation doesn't change the query, we've wasted cpu to make this copy
    // and this is likely the majority of the cases
    // TODO: optimize: a better approach here would be to use CoW
    type_query apply_mutation_rules_l(type_mutation& mutation, const mutation_rule_map& rules) {

        // so, rules may have dependencies between themselves
        // it would be easy to say "users must take care of them by order_priority" and dynamix 1 did just that
        // however since we allow adding rules with mixin registration and potentially adding of rules
        // by plugins this task would be really hard for users
        // so we help by doing a poor man's toplogical sort: multiple applications
        // we can't do a real topological sort, because rules are opaque
        // (and there is no obvious way to make them non-opaque, while keeping their power)
        // so, we run them again and again until applying them produces the same result as the previous apply
        // but no more than...
        static constexpr int max_depenency_depth = 5; // somewhat arbitrary, but who needs more than 5? :)
        // ...times. It's true that deeper non-cycling dependencies may exist,
        // but we risk staying it this function for way too long.
        // instead we give this arbitrary limit and say: "users must take care to sort rules with
        // deeper dependencies by order_priority": a much laxer restriction than the initial one

        type_query original_query(m_allocator); // store original query here to return
        type_query last_result(m_allocator); // store last rule application result here

        auto& nt_mixins = mutation.mixins;
        last_result = nt_mixins;

        // first erase all deps from mixins
        itlib::erase_all_if(nt_mixins, [](const mixin_info* m) { return m->dependency; });

        for (int i = 0; ; ++i) {
            // apply rules
            for (auto& r : rules) {
                auto& info = *r.first;
                if (auto err = info.apply(mutation.to_c_hanlde(), info.user_data)) {
                    throw_exception::mutation_rule_user_error(mutation, info, err);
                }
            }
            if (last_result == nt_mixins) {
                // rules changed nothing, so no need to apply them again
                if (i == 0) return last_result; // first application changed nothing
                return original_query;
            }
            if (i == max_depenency_depth || i == int(rules.size())) {
                // again: this is not necessarily a real cycle but we treat it as such
                throw_exception::cyclic_rule_deps(mutation);
            }
            if (i == 0) {
                // first time running the loop and rules made changes, store original query to return
                original_query.swap(last_result);
            }
            last_result = nt_mixins;
        }
    }

    const type& get_type(type_mutation& mutation) {
        if (&mutation.dom != &m_domain) throw_exception::foreign_mutation(m_domain, mutation);

        type_query original_query(m_allocator); // prepare original query with our allocator
        const type* found = nullptr;

        {
            // search for stored query for this combo
            auto reg = m_type_registry.shared_lock();
            {
                auto f = reg->type_queries.find(mutation.mixins);
                if (f != reg->type_queries.end()) return *f->second;
            }

            // query is not available, so we need to apply mutation rules
            // we can do it while holding the shared lock
            original_query = apply_mutation_rules_l(mutation, reg->mutation_rules);

            // now look for exact type
            if (mutation.mixins.empty()) {
                found = &m_empty_type;
            }
            else {
                auto f = reg->types.find(mutation.mixins);
                if (f != reg->types.end()) found = f->get();
            }
        }

        if (found) {
            // we found a type with this mutation, but don't have the query stored
            // store the query only and return the type

            // it may be the case here that another thread waits to do the exact same thing
            // but the code below is safe
            // worst (and extremely rare) case we wasted cpu applying the same rules twice

            auto reg = m_type_registry.unique_lock();
            reg->type_queries[std::move(original_query)] = found;
            return *found;
        }

        return create_type(mutation, std::move(original_query));
    }

    const type& get_type(itlib::span<const mixin_info* const> mixins) {
        {
            // search for stored query for this combo
            auto reg = m_type_registry.shared_lock();
            auto f = reg->type_queries.find(mixins);
            if (f != reg->type_queries.end()) return *f->second;
        }

        // no stored query, so create a mutation and apply rules
        // creating a mutation will run more or less the exact same as above again
        // TODO: optimize
        type_mutation mut(m_domain, m_allocator);
        mut.mixins.assign(mixins.begin(), mixins.end());
        return get_type(mut);
    }

    struct ftable_build_helper {
        const type_mutation& m_mut;

        // number of reachable payloads per feature id
        compat::pmr::vector<uint32_t> m_num_reachable_pls;

        // total number of ftable_payload-s to allocate
        uint32_t m_num_total_pls = 0;

        ftable_build_helper(type_mutation& mutation)
            : m_mut(mutation)
            , m_num_reachable_pls(m_mut.dom.get_allocator())
        {
            // first pass: find greatest feature_id in mixins to determine ftable size
            feature_id::int_t ftable_size = 0;
            for (const auto* mixin : m_mut.mixins) {
                for (auto& feature : mixin->features_span()) {
                    const auto size_with = feature.info->iid() + 1;
                    if (size_with > ftable_size) {
                        ftable_size = size_with;
                    }
                }
            }

            // second pass:
            // * count reachable payloads per feature
            // * count total payloads
            m_num_reachable_pls.resize(ftable_size);
            for (const auto* mixin : m_mut.mixins) {
                for (auto& feature : mixin->features_span()) {
                    ++m_num_reachable_pls[feature.info->iid()];
                    ++m_num_total_pls;
                }
            }
        }

        byte_size_t calc_ftable_byte_size() const noexcept {
            return byte_size_t(m_num_reachable_pls.size() * sizeof(type::ftable_entry) + m_num_total_pls * sizeof(type::ftable_payload));
        }

        // we need a mutable entry while we build
        // we could const_cast, but it's easier with a new type
        struct mutable_ftable_entry {
            type::ftable_payload* begin;
            type::ftable_payload* top_bid_back;
            type::ftable_payload* end;
            explicit operator bool() const noexcept { return !!begin; }
        };

        [[maybe_unused]] static void _ensure_ftable_entry_compat() {
            static_assert(sizeof(mutable_ftable_entry) == sizeof(type::ftable_entry));
            mutable_ftable_entry me = {};
            type::ftable_entry ve = {};
            ve.begin = me.begin;
            static_assert(sizeof(ve.begin == me.begin));
            ve.end = me.end;
            static_assert(sizeof(ve.end == me.end));
            static_assert(offsetof(mutable_ftable_entry, end) == offsetof(type::ftable_entry, end));
        }

        itlib::span<const type::ftable_entry> build_ftable(byte_t* const ptr) const {
            std::fill(ptr, ptr + calc_ftable_byte_size(), byte_t{0});
            itlib::span<mutable_ftable_entry> ftable(reinterpret_cast<mutable_ftable_entry*>(ptr), m_num_reachable_pls.size());
            auto ftable_pl_ptr = reinterpret_cast<type::ftable_payload*>(ptr + ftable.size_bytes());

            // third pass
            // attach begin and end pointers of ftable entries and give them values
            for (mixin_index_t i = 0; i < m_mut.mixins.size(); ++i) {
                const auto* mixin = m_mut.mixins[i];
                for (auto& feature : mixin->features_span()) {
                    auto& finfo = *feature.info;
                    auto& entry = ftable[finfo.iid()];
                    const auto num_reachable = m_num_reachable_pls[finfo.iid()];

                    // there must be at least one
                    // after all we're moving through provided features
                    assert(num_reachable != 0);

                    if (!entry) {
                        entry.begin = ftable_pl_ptr;
                        entry.top_bid_back = entry.begin;
                        entry.end = entry.begin;
                        ftable_pl_ptr += num_reachable;
                    }

                    entry.end->mixin_index = i;
                    entry.end->payload = feature.payload;
                    entry.end->data = &feature;
                    ++entry.end;
                }
            }

            // fourth and final pass
            // * sort ranges
            // * check for clashes
            // * fix top_bid_back pointers
            for (auto& entry : ftable) {
                if (!entry) continue; // not implemented

                std::sort(entry.begin, entry.end, [&](const type::ftable_payload& a, const type::ftable_payload& b) {
                    auto& adata = *a.data;
                    auto& bdata = *b.data;

                    // first by bid
                    if (adata.bid != bdata.bid) return adata.bid > bdata.bid;

                    // then by prio
                    if (adata.priority != bdata.priority) return adata.priority < bdata.priority;

                    // then by mixin order such that the last mixin is first
                    return a.mixin_index > b.mixin_index;
                });

                // check for clashes
                if (!entry.begin->data->info->allow_clashes) {
                    for (auto ie = entry.begin; ie != entry.end - 1; ++ie) {
                        const auto& cur = *ie;
                        const auto& next = *(ie + 1);
                        if (cur.data->bid == next.data->bid && cur.data->priority == next.data->priority) {
                            // same bid and prio = clash
                            throw_exception::feature_clash(m_mut, cur, next);
                        }
                    }
                }

                // set top_bid_back pointer (currently pointing to begin)
                for (auto pl = entry.begin + 1; pl != entry.end; ++pl) {
                    if (pl->data->bid != entry.top_bid_back->data->bid) {
                        // top bid end reached
                        break;
                    }
                    entry.top_bid_back = pl;
                }
            }

            return itlib::span<const type::ftable_entry>(reinterpret_cast<type::ftable_entry*>(ptr), m_num_reachable_pls.size());
        }
    };

    // create type for a given mutation requested by a given query
    const type& create_type(type_mutation& mutation, type_query&& query) {
        mixin_info_span mixins(mutation.mixins);

        // first check validity
        for (size_t i = 0; i < mixins.size(); ++i) {
            auto m = mixins[i];
            if (m->id == invalid_mixin_id) throw_exception::type_mut_error(mutation, "unregistered", *m);
            if (m->dom != &m_domain) throw_exception::foreign_mixin(mutation, *m);
            for (size_t j = i + 1; j < mixins.size(); ++j) {
                if (mixins[i] == mixins[j]) throw_exception::type_mut_error(mutation, "duplicate", *m);
            }
        }

        // so, we need to create a new obj type...

        // we allocate a single buffer in the type
        // this is the order in which elements are placed inside
        // to avoid manually fixing the alignment, we check here that the alignments are in a non-increasing order
        // (ie they will be fixed by the compiler)
        static_assert(std::is_trivially_destructible_v<type>);
        static_assert(alignof(type) >= alignof(typename type::ftable_entry), "fix type buffer");
        static_assert(alignof(typename type::ftable_entry) >= alignof(typename type::ftable_payload), "fix type buffer");
        static_assert(alignof(typename type::ftable_payload) >= alignof(void*), "fix type buffer");
        static_assert(alignof(void*) >= alignof(uint32_t), "fix type buffer");
        static_assert(alignof(uint32_t) >= alignof(mixin_index_t), "fix type buffer");
        static_assert(std::is_trivial_v<type::ftable_entry>, "fix type buffer");

        // prepare single buffer for type

        // calc buf components
        const byte_size_t type_size = sizeof(type);

        const ftable_build_helper ftable_helper(mutation);
        const byte_size_t ftable_size = ftable_helper.calc_ftable_byte_size();

        const byte_size_t mixins_buf_size = byte_size_t(mixins.size_bytes());

        const byte_size_t mixin_offsets_buf_size = byte_size_t(mixins.size() * sizeof(uint32_t));

        const auto num_sparse = /*iile*/[&]() {
            auto max_by_id = std::max_element(mixins.begin(), mixins.end(), [](const mixin_info* a, const mixin_info* b) {
                return a->iid() < b->iid();
            });
            return (*max_by_id)->iid() + 1;
        }();
        const byte_size_t sparse_mixin_indices_buf_size = num_sparse * sizeof(mixin_index_t);

        //const byte_size_t type_classes_buf_size = byte_size_t(m_domain.m_type_classes.size() * sizeof(bool));

        // alloc and fill buf
        const byte_size_t total_obj_type_buf_size =
            type_size
            + ftable_size
            + mixins_buf_size
            + mixin_offsets_buf_size
            + sparse_mixin_indices_buf_size;

        auto new_type_bytes = reinterpret_cast<byte_t*>(m_allocator.allocate_bytes(
            total_obj_type_buf_size,
            alignof(type)
        ));
        uptr<type> new_type(new (new_type_bytes) type(m_domain, total_obj_type_buf_size));
        auto* bptr = new_type_bytes + sizeof(type);

        // ftable
        auto ftable = ftable_helper.build_ftable(bptr);
        bptr += ftable_size;
        new_type->ftable = ftable.data();
        new_type->ftable_length = uint32_t(ftable.size());

        // mixins
        itlib::span new_type_mixins(reinterpret_cast<const mixin_info**>(bptr), mixins.size());
        bptr += mixins_buf_size;
        std::copy(mixins.begin(), mixins.end(), new_type_mixins.begin());
        new_type->mixins = new_type_mixins;

        // object buffer data

        // the single buffer is structured as follows
        // - an array of object_mixin_data for each mixin
        // - maybe padding
        // - internal mixin [0] object pointer
        // - internal mixin [0] buffer
        // - maybe padding ...
        // - internal mixin [1] object pointer
        // - internal mixin [1] buffer
        // ...
        static_assert(std::is_trivial_v<object_mixin_data>);
        itlib::span mixin_offsets(reinterpret_cast<uint32_t*>(bptr), mixins.size());
        bptr += mixin_offsets_buf_size;
        new_type->mixin_offsets = mixin_offsets;

        static_assert(alignof(object_mixin_data) == sizeof_ptr);
        byte_size_t obj_buf_size = byte_size_t(mixins.size() * sizeof(object_mixin_data)); // base size
        byte_size_t obj_buf_alignment = sizeof_ptr; // base alignment
        for (size_t i = 0; i < mixins.size(); ++i) {
            const auto* m = mixins[i];
            auto& mo = mixin_offsets[i];
            if (m->external()) {
                mo = 0;
                continue; // externals don't affect alignment or size
            }
            auto ma = m->alignment;
            if (ma > obj_buf_alignment) obj_buf_alignment = ma;
            else if (ma < sizeof_ptr) ma = sizeof_ptr;

            auto padding = util::next_multiple(obj_buf_size, ma) - obj_buf_size;
            if (padding < sizeof_ptr) padding += ma;

            obj_buf_size += padding;
            assert(obj_buf_size % ma == 0);

            mo = obj_buf_size; // mixin offset is here

            obj_buf_size += m->size;
        }
        new_type->object_buffer_size = obj_buf_size;
        new_type->object_buffer_alignment = obj_buf_alignment;

        // sparse indices
        itlib::span sparse_mixin_indices(reinterpret_cast<mixin_index_t*>(bptr), num_sparse);
        bptr += sparse_mixin_indices_buf_size;
        std::fill(sparse_mixin_indices.begin(), sparse_mixin_indices.end(), invalid_mixin_index);
        for (mixin_index_t i = 0; i < mixins.size(); ++i) {
            sparse_mixin_indices[mixins[i]->iid()] = i;
        }
        new_type->sparse_mixin_indices = sparse_mixin_indices;

        // finally add new type to types and return it
        auto reg = m_type_registry.unique_lock();
        auto res = reg->types.emplace(std::move(new_type));

        // note that the type may already be added
        // could be more than one thread waited at the mutex above for the exact same type
        // in which case we give up on our own
        // it may seem like a waste to have more than one thread create the exact same type
        // before waiting on the unique_lock
        // it may seem to be a good idea to lock earlier, but this should be very very rare
        // we're willing to risk dropping materialized types every once in a blue moon
        // for the benefit of holding the unique_lock for as short amount of time as possible
        // in any case we can disregard res.second and just register the query
        // (the query may also be the same as the one from the previous thread,
        // but the code below is safe in such a case)

        const type* reg_type = res.first->get();
        reg->type_queries[std::move(query)] = reg_type;
        return *reg_type;
    }

    void garbage_collect_types() noexcept {
        auto l = m_type_registry.unique_lock();
        auto& types = l->types;
        auto& queries = l->type_queries;
        for (auto it = types.begin(); it != types.end(); ) {
            auto& t = *it;
            if (t->num_objects() > 0) {
                // objects of this type still exist
                ++it;
                continue;
            }

            // erase queries which lead to this type
            for (auto iq = queries.begin(); iq != queries.end(); ) {
                if (iq->second == it->get()) {
                    iq = queries.erase(iq);
                }
                else {
                    ++iq;
                }
            }
            it = types.erase(it);
        }
    }
};

struct domain_traverse::impl {
    data_mutex<domain::impl::type_registry>::shared_lock_t tr;
    data_mutex<domain::impl::element_registry>::shared_lock_t er;
};

domain_traverse::domain_traverse(const domain& d) noexcept {
    m_impl = new impl{
        d.m_impl->m_type_registry.shared_lock(),
        d.m_impl->m_element_registry.shared_lock()
    };
}
domain_traverse::~domain_traverse() {
    delete m_impl;
}

void domain_traverse::traverse_mixins(std::function<void(const mixin_info&)> func) const {
    for (auto* m : m_impl->er->sparse_mixins) {
        if (m) func(*m);
    }
}
void domain_traverse::traverse_features(std::function<void(const feature_info&)> func) const {
    for (auto* f : m_impl->er->sparse_features) {
        if (f) func(*f);
    }
}
void domain_traverse::traverse_mutation_rules(std::function<void(const mutation_rule_info&, uint32_t)> func) const {
    for (auto& mr : m_impl->tr->mutation_rules) {
        func(*mr.first, mr.second);
    }
}
void domain_traverse::traverse_type_classes(std::function<void(const type_class&)> func) const {
    for (auto* tc : m_impl->er->sparse_type_classes) {
        if (tc) func(*tc);
    }
}
void domain_traverse::traverse_types(std::function<void(const type&)> func) const {
    for (auto& t : m_impl->tr->types) {
        func(*t);
    }
}
void domain_traverse::traverse_type_queries(std::function<void(itlib::span<const mixin_info* const>, const type&)> func) const {
    for (auto& tc : m_impl->tr->type_queries) {
        func(tc.first, *tc.second);
    }
}

domain::domain(std::string_view name, domain_settings settings, uintptr_t user_data, void* context, allocator alloc)
    : dnmx_basic_domain{dnmx_sv::from_std(name), settings, user_data, context}
    , m_impl(new impl(*this, alloc))
{}

domain::~domain() {
    delete m_impl;
}


void domain::register_mixin(mixin_info& info) {
    m_impl->register_mixin(info);
}


void domain::unregister_mixin(mixin_info& info) {
    m_impl->unregister_mixin(info);
}

const mixin_info* domain::get_mixin_info(mixin_id id) noexcept {
    return m_impl->get_mixin_info(id);
}

const mixin_info* domain::get_mixin_info(std::string_view name) noexcept {
    return m_impl->get_mixin_info(name);
}

void domain::register_feature(feature_info& info) {
    m_impl->register_feature(info);
}

void domain::unregister_feature(feature_info& info) {
    m_impl->unregister_feature(info);
}

const feature_info* domain::get_feature_info(feature_id id) noexcept {
    return m_impl->get_feature_info(id);
}

const feature_info* domain::get_feature_info(std::string_view name) noexcept {
    return m_impl->get_feature_info(name);
}

void domain::register_type_class(const type_class& tc) {
    m_impl->register_type_class(tc);
}

void domain::unregister_type_class(const type_class& tc) {
    m_impl->unregister_type_class(tc);
}

const type_class* domain::get_type_class(std::string_view name) noexcept {
    return m_impl->get_type_class(name);
}

void domain::add_mutation_rule(const mutation_rule_info& info) {
    m_impl->add_mutation_rule(info);
}
void domain::remove_mutation_rule(const mutation_rule_info& info) noexcept {
    m_impl->remove_mutation_rule(info);
}

const type& domain::get_type(type_mutation&& mutation) {
    return m_impl->get_type(mutation);
}

const type& domain::get_type(itlib::span<const mixin_info* const> mixins) {
    return m_impl->get_type(mixins);
}

allocator domain::get_allocator() const noexcept {
    return m_impl->m_allocator;
}

const type& domain::get_empty_type() const noexcept {
    return m_impl->m_empty_type;
}

size_t domain::num_types() const noexcept {
    return m_impl->m_type_registry.shared_lock()->types.size();
}

// performs garbage collection removing object types with zero objects
void domain::garbage_collect_types() noexcept {
    return m_impl->garbage_collect_types();
}

size_t domain::num_type_queries() const noexcept {
    return m_impl->m_type_registry.shared_lock()->type_queries.size();
}

size_t domain::num_mutation_rules() const noexcept {
    return m_impl->m_type_registry.shared_lock()->mutation_rules.size();
}

}
