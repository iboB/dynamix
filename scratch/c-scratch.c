// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dnmx/domain.h>
#include <dnmx/feature_info.h>
#include <dnmx/mixin_info.h>

#include <stdio.h>

void feature(void) {
    dnmx_feature_info
        f1 = dnmx_make_feature_info(),
        f2 = dnmx_make_feature_info();

    f1.name = dnmx_make_sv_lit("asdf");
    f2.name = dnmx_make_sv_lit("asdf");

    dnmx_domain_handle dom = dnmx_create_domain(dnmx_make_sv_lit("test"), (dnmx_domain_settings) { 0 }, 0, NULL);
    dnmx_register_feature(dom, &f1);

    // the next call fails, because because f2 has the same name as an already registered feature
    dnmx_register_feature(dom, &f2);

    {
        // repro with string which was used before throwing the exception
        dnmx_sv sv = dnmx_make_sv_lit("asdf");
        printf("%zu\n", sv.end - sv.begin);
    }

    {
        // no repro with string which was used before throwing the exception
        dnmx_sv sv = dnmx_make_sv_lit("qwer");
        printf("%zu\n", sv.end - sv.begin);
    }

    dnmx_destroy_domain(dom);
}

int main(void) {
    feature();
    return 0;
}
