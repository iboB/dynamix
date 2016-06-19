// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "common.hpp"
#include "generated.hpp"

#include "custom_alloc_mixin.hpp"

#include <iostream>

#include "../common/timer.hpp"

using namespace std;
using namespace dynamix;

const size_t NUM_TO_MUTATE = 100000;

int main(int argc, char**)
{
    std::vector<object_type_template*> templates;
    templates.reserve(2000);

    timer t;

    create_type_templates(templates);

    const size_t num_tmpl = templates.size();

    vector<object*> objects(num_tmpl);

    for(size_t i=0; i<num_tmpl; ++i)
    {
        objects[i] = new object;
    }

    // create objects from type templates
    t.start("New type");
        for(size_t i=0; i<num_tmpl; ++i)
        {
            templates[i]->apply_to(*objects[i]);
        }
    t.avg(num_tmpl);

    vector<object_type_template*> changable_templates;

    // collect all type templates which create object that
    // would change from such a mutation
    for(size_t i=0; i<num_tmpl; ++i)
    {
        const object* o = objects[i];

        if(!o->has<mixin_1>() || !o->has<mixin_2>() || o->has<mixin_3>())
        {
            changable_templates.push_back(templates[i]);
        }
    }

    // create a huge amount of objects
    vector<object*> to_mutate(NUM_TO_MUTATE);
    for(size_t i=0; i<NUM_TO_MUTATE; ++i)
    {
        to_mutate[i] = new object;
    }

    // create objects of existing types
    t.start("Existing type");
        for(size_t i=0; i<NUM_TO_MUTATE; ++i)
        {
            changable_templates[i%changable_templates.size()]->apply_to(*to_mutate[i]);
        }
    t.avg(NUM_TO_MUTATE);


    // mutate them
    // mutating objects
    t.start("Mutate");
        for(size_t i=0; i<NUM_TO_MUTATE; ++i)
        {
            mutate(to_mutate[i])
                .add<mixin_1>()
                .add<mixin_2>()
                .remove<mixin_3>();

            mutate(to_mutate[i])
                .remove<mixin_1>()
                .add<mixin_3>();
        }
    t.avg(NUM_TO_MUTATE * 2);

    // create 1 million objects of the same type
    object_type_template tmpl;
    tmpl
        .add<mixin_3>()
        .add<mixin_4>()
        .add<mixin_5>()
        .create();

    vector<object*> same_type_objs(NUM_TO_MUTATE);
    for(size_t i=0; i<NUM_TO_MUTATE; ++i)
    {
        same_type_objs[i] = new object(tmpl);
    }

    same_type_mutator stm;
    stm
        .add<mixin_1>()
        .add<mixin_2>()
        .remove<mixin_3>();

    // mutating objects of the same type
    t.start("Same type mutate");
        for(size_t i=0; i<NUM_TO_MUTATE; ++i)
        {
            stm.apply_to(*same_type_objs[i]);
        }
    t.avg(NUM_TO_MUTATE);

    // adding a mixin which has a custom allocator to the objects
    t.start("Custom allocator");
        for(size_t i=0; i<NUM_TO_MUTATE; ++i)
        {
            mutate(to_mutate[i])
                .add<custom_alloc_mixin>();
        }
    t.avg(NUM_TO_MUTATE);

    same_type_mutator stm_custom_alloc;
    stm_custom_alloc.add<custom_alloc_mixin>();

    // adding a mixin with custom allocator to the objects of same type
    t.start("Same type alloc");
        for(size_t i=0; i<NUM_TO_MUTATE; ++i)
        {
            stm_custom_alloc.apply_to(*same_type_objs[i]);
        }
    t.avg(NUM_TO_MUTATE);

    return 0;
}
