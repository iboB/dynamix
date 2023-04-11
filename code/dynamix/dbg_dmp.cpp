// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "dbg_dmp.hpp"
#include "domain.hpp"
#include "domain_traverse.hpp"
#include "feature_info.hpp"
#include "mixin_info.hpp"
#include "mutation_rule_info.hpp"
#include "type.hpp"
#include "object.hpp"

#include <ostream>

namespace dynamix::util {
namespace {
struct p {
	template <typename T>
    p(T* pp) : ptr(reinterpret_cast<const void*>(pp)) {}
    const void* ptr;
};
std::ostream& operator<<(std::ostream& out, p pp) {
    if (pp.ptr) {
        out << pp.ptr;
    }
    else {
        out << "none";
    }
    return out;
}

#define IND "  "
#define NIND "\n  "

void output_mixins(std::ostream& out, const itlib::span<const mixin_info* const>& mixins) {
    out << '{';
    bool first = true;
    for (const auto& m : mixins) {
        if (first) {
            first = false;
        }
        else {
            out << ", ";
        }
        out << '\'' << m->name.to_std() << '\'';
    }
    out << '}';
}

void dbg_dmp_l(std::ostream& out, const type& t, uint32_t flags, domain_traverse* tr) {
    out << "type: ";
    output_mixins(out, t.mixins);
    out << " (" << &t << ") ('" << t.dom.name() << "')";

    int num_features = 0;
    itlib::span ftable(t.ftable, t.ftable_length);
    for (auto& f : ftable) num_features += !!f;

    if (flags & dnmx_type_dmp_ex) {
        out << NIND "size: " << t.buf_size();
        out << NIND "obj buf size: " << t.object_buffer_size;
        out << NIND "obj buf align: " << t.object_buffer_alignment;
        out << NIND "num obj: " << t.num_objects();
        out << NIND "default_constructible: " << t.default_constructible();
        out << NIND "copy_constructible: " << t.copy_constructible();
        out << NIND "copy_assignable: " << t.copy_assignable();
        out << NIND "equality_comparable: " << t.equality_comparable();
        out << NIND "comparable: " << t.comparable();
        out << NIND "features: " << num_features << '/' << ftable.size();
    }
    if (flags & dnmx_type_dmp_ftable) {
        out << NIND << num_features << " features:";
        for (auto& f : ftable) {
            if (!f) continue;
            out << NIND IND "impl: '" << f.begin->data->info->name.to_std() << "' " << (f.top_bid_back - f.begin) + 1 << '/' << f.size();
            if (flags & dnmx_type_dmp_full_ftable) {
                for (auto e = f.begin; e != f.end; ++e) {
                    out << NIND IND IND "'" << t.mixins[e->mixin_index]->name.to_std() << "': " << p(e->payload) << " with " << e->data->bid << "_bid " << e->data->priority << "_prio";
                }
            }
        }
    }
    if (tr && (flags & dnmx_type_dmp_matching_queries)) {
        int count = 0;
        tr->traverse_type_queries([&](itlib::span<const mixin_info* const>, const type& qt) {
            if (qt == t) count += 1;
        });
        out << NIND << count << " matching queries:";
        tr->traverse_type_queries([&](itlib::span<const mixin_info* const> mixins, const type& qt) {
            if (qt != t) return;
            out << NIND IND;
            output_mixins(out, mixins);
        });
    }
    if (tr && (flags & dnmx_type_dmp_matching_type_classes)) {
        out << NIND "matching type classes:";
        tr->traverse_type_classes([&](const type_class& tc) {
            if (!tc.matches(&t)) return;
            out << NIND IND << tc.name.to_std();
        });
    }
}

void dbg_dmp_l(std::ostream& out, const type_class& tc, uint32_t flags, domain_traverse* tr) {
    out << "type class: '" << tc.name.to_std() << ", match: " << p(tc.matches);
    if (tc.matches && tr && (flags & dnmx_tc_dmp_matching_types)) {
        tr->traverse_types([&](const type& t) {
            if (!tc.matches(&t)) return;
            out << "match ";
            dbg_dmp(out, t, 0);
        });
    }
}
}

void dbg_dmp(std::ostream& out, const domain& d, uint32_t flags) {
    domain_traverse tr(d);
    out << std::boolalpha;
    out << "domain: '" << d.name() << "' (" << &d << "), ud: " << d.user_data << ", ctx : ";
    if (d.context) {
        out << d.context;
    }
    else {
        out << "<null>";
    }
    out << ", alloc: ";
    allocator check;
    {
        auto da = d.get_allocator();
        if (da == check) {
            out << "default";
        }
        else {
            out << "mr " << da.resource();
        }
    }
    out << '\n';
    if (flags & dnmx_dom_dmp_settings) {
        auto& s = d.settings();
        out << "## Settings";
        out << NIND "canonicalize_types: " << s.canonicalize_types;
        out << NIND "allow_duplicate_feature_names: " << s.allow_duplicate_feature_names;
        out << NIND "allow_duplicate_mixin_names: " << s.allow_duplicate_mixin_names;
        out << '\n';
    }
    if (flags & dnmx_dom_dmp_features) {
        int count = 0;
        tr.traverse_features([&](const feature_info&) { ++count; });

        out << "## " << count << " Features\n";
        feature_id::int_t next_slot = 0;
        tr.traverse_features([&](const feature_info& fi) {
            if (fi.iid() != next_slot) {
                out << fi.iid() - next_slot << " empty slots\n";
            }

            dbg_dmp(out, fi, flags);
            out << '\n';

            next_slot = fi.iid() + 1;
        });
    }
    if (flags & dnmx_dom_dmp_mixins) {
        int count = 0;
        tr.traverse_mixins([&](const mixin_info&) { ++count; });

        out << "## " << count << " Mixins\n";
        mixin_id::int_t next_slot = 0;
        tr.traverse_mixins([&](const mixin_info& mi) {
            if (mi.iid() != next_slot) {
                out << mi.iid() - next_slot << " empty slots\n";
            }

            dbg_dmp(out, mi, flags);
            out << '\n';

            next_slot = mi.iid() + 1;
        });
    }
    if (flags & dnmx_dom_dmp_mutation_rules) {
        out << "## " << d.num_mutation_rules() << " Mutation Rules\n";
        tr.traverse_mutation_rules([&](const mutation_rule_info& mri, uint32_t refs) {
            dbg_dmp(out, mri, flags);
            out << ", " << refs << " refs\n";
        });
    }
    if (flags & dnmx_dom_dmp_types) {
        out << "## " << d.num_types() << " Types\n";
        tr.traverse_types([&](const type& t) {
            dbg_dmp_l(out, t, flags, &tr);
            out << '\n';
        });
    }
    if (flags & dnmx_dom_dmp_type_classes) {
        int count = 0;
        tr.traverse_type_classes([&](const type_class&) { ++count; });

        out << "## " << count << " Type Classes\n";
        tr.traverse_type_classes([&](const type_class& tc) {
            dbg_dmp(out, tc, flags);
            out << '\n';
        });
    }
    if (flags & dnmx_dom_dmp_type_queries) {
        out << "## " << d.num_type_queries() << " Type Queries\n";
        tr.traverse_type_queries([&](itlib::span<const mixin_info* const> mixins, const type& qt) {
            out << "tq: ";
            output_mixins(out, mixins);
            out << " -> type: ";
            output_mixins(out, qt.mixins);
            out << '\n';
        });
    }
}

void dbg_dmp(std::ostream& out, const mixin_info& mi, uint32_t flags) {
    out << std::boolalpha;
    if (mi.dependency) {
        out << "dep-";
    }
    out << "mixin: '" << mi.name.to_std() << "', ";
    if (mi.id == invalid_mixin_id) {
        out << "unregistered";
    }
    else {
        out << "id: " << mi.iid() << " ('" << mi.dom->m_name.to_std() << "')";
    }
    out << ", ";
    if (mi.external()) {
        out << "external";
    }
    else {
        out << "internal";
    }
    auto features = mi.features_span();
    out << ", " << features.size() << " features";
    if (flags & dnmx_mixin_dmp_ex) {
        out << NIND "size: " << mi.size;
        out << NIND "alignment: " << mi.alignment;
        out << NIND "obj_buf_size: " << mi.obj_buf_size;
        out << NIND "obj_buf_align: " << mi.obj_buf_alignment_and_mixin_offset;
        out << NIND "init: " << p(mi.init);
        out << NIND "copy_init: " << p(mi.copy_init);
        out << NIND "move_init: " << p(mi.move_init);
        out << NIND "copy_asgn: " << p(mi.copy_asgn);
        out << NIND "move_asgn: " << p(mi.move_asgn);
        out << NIND "destroy: " << p(mi.destroy);
        out << NIND "equals: " << p(mi.equals);
        out << NIND "compare: " << p(mi.compare);
        out << NIND "allocator: " << p(mi.allocator);
        out << NIND "class: " << mi.mixin_class;
        out << NIND "type_info_class: " << mi.type_info_class;
        out << NIND "user_data: " << mi.user_data;
        out << NIND "order_priority: " << mi.order_priority;
        out << NIND "force_external: " << mi.force_external;
    }
    if (flags & dnmx_mixin_dmp_features) {
        out << NIND << features.size() << " features:";
        for (auto& f : features) {
            out << NIND IND "'" << f.info->name.to_std() << "': " << p(f.payload) << ' '
                << f.bid << "_bid " << f.priority << "_prio";
        }
    }
}

void dbg_dmp(std::ostream& out, const feature_info& fi, uint32_t) {
    out << std::boolalpha;
    out << "feature: '" << fi.name.to_std() << "', ";
    if (fi.id == invalid_feature_id) {
        out << "unregistered";
    }
    else {
        out << "id: " << fi.iid();
    }
    out << ", allow clashes: " << fi.allow_clashes << ", class: "
        << fi.feature_class << ", default payload: " << p(fi.default_payload);
}

void dbg_dmp(std::ostream& out, const mutation_rule_info& mri, uint32_t) {
    out << "rule: '" << mri.name.to_std() << "', prio: " << mri.order_priority
        << ", ud: " << mri.user_data << ", apply: " << p(mri.apply);
}

void dbg_dmp(std::ostream& out, const type& t, uint32_t flags) {
    if (flags & (dnmx_type_dmp_matching_type_classes | dnmx_type_dmp_matching_queries)) {
        domain_traverse tr(t.dom);
        dbg_dmp_l(out, t, flags, &tr);
    }
    else {
        dbg_dmp_l(out, t, flags, nullptr);
    }
}

void dbg_dmp(std::ostream& out, const type_class& tc, uint32_t flags) {
    dbg_dmp_l(out, tc, flags, nullptr);
}
}
