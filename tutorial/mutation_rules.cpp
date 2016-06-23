// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>
#include <iostream>

using namespace std;

//[tutorial_mutation_rules_A
/*`
Let's define some mixins that may be present in a CAD system specialized for
furniture design. Like in the previous example, we won't concern ourselves with any
particular messages.

So, again we have a mixin that we want to be present in every object.
*/

class furniture
{
    int _id;
    string name;
    // ... other common fields
};

DYNAMIX_DEFINE_MIXIN(furniture, dynamix::none);

/*`
We also have mixins that describe the frame of the piece of furniture.
*/

class wood_frame {};
DYNAMIX_DEFINE_MIXIN(wood_frame, dynamix::none);

class metal_frame {};
DYNAMIX_DEFINE_MIXIN(metal_frame, dynamix::none);

/*`
Let's also define some mixins that will be responsible for the object
serialization.
*/

class ofml_serialization {};
DYNAMIX_DEFINE_MIXIN(ofml_serialization, dynamix::none);

class xml_serialization {};
DYNAMIX_DEFINE_MIXIN(xml_serialization, dynamix::none);

/*`
And finally let's define two mixins that would help us describe our piece of
furniture if it can contain objects inside -- like a cabinet or a wardrobe.
*/

class has_doors {};
DYNAMIX_DEFINE_MIXIN(has_doors, dynamix::none);

class container {};
DYNAMIX_DEFINE_MIXIN(container, dynamix::none);

//]

int main()
{
//[tutorial_mutation_rules_B

/*`
Now, let's move on to the entry point of our program.

We said that each and every object in our system should be expected to have the
mixin `furniture`. That could be accomplished if we manually add it to all
mutations we make but there is a simpler way to do it. By adding the
`mandatory_mixin` mutation rule.

All mutation rules should be added by calling `add_new_mutation_rule`. Since
`mandatory_mixin` is a mutation rule that the library provides, we can
accomplish this with a single line of code:
*/
    dynamix::add_new_mutation_rule(new dynamix::mandatory_mixin<furniture>);
/*`
Now each mutation after this line will add `furniture` to the objects (even if
it's not been explicitly added) and also if a mutation tries to remove the `furniture` mixin
from the object, it won't be allowed. There won't be an error or a warning. The
library will silently ignore the request to remove `furniture`, or any other
mixin that's been added as mandatory. Note, that if a mutation tries to remove
`furniture`, and also adds and removes other mixins, only the part removing the
mandatory mixin will be ignored. The others will be performed.

Another common case for using `mandatory_mixin` is if you want to have some
debugging mixin, that you want present in you objects, when you're debugging
your application. This is very easily accomplished if you just set the rule for
it in a conditional compilation block.

You probably noticed the mixin `ofml_serialization`. OFML is a format
specifically designed for describing furniture that's still used in some
European countries, but hasn't gotten worldwide acceptance. Let's assume we want
to drop the support for OFML, but without removing the actual code, since
third party plugins to our CAD system may still depend on it. All we want is to
prevent anybody from adding the mixin to an object. Basically the exact opposite
of `mandatory_mixin`. This is the mutation rule `deprecated_mixin`
*/

    dynamix::add_new_mutation_rule(new dynamix::deprecated_mixin<ofml_serialization>);
/*`
After that line of code, any mutation that tries to add
`ofml_serialization` won't be able to, and all mutations will try to remove it
if it's present in an object. Again, as was the case before, if a mutation does
many things, only the part from it, trying to add `ofml_serialization` will be
silently ignored.

The last built-in rule in the library is `mutually_exclusive_mixins`.

Since a piece of furniture has either wood frame or a metal frame and never
both, it would be a good idea to prevent the programmers from accidentally
adding both mixins representing the frame in a single object. This mutation rule
helps us do exactly that.
*/

    dynamix::mutually_exclusive_mixins* rule = new dynamix::mutually_exclusive_mixins;
    rule->add<wood_frame>();
    rule->add<metal_frame>();
    dynamix::add_new_mutation_rule(rule);

/*`
You may add as many mutually exclusive mixins as you wish. If you had, say,
`plastic_frame`, you could also add it to that list.

Any object mutated after that rule is set will implicitly remove any of the
mutually exclusive mixins if another is added.

In many of our examples a sample game code was given, with mixins
`opengl_rendering` and `directx_rendering`. The `mutually_exclusive_mixins` is
perfect for this case and any other when we're always doing
`add<x>().remove<y>()` and `add<y>().remove<x>()`.

So to see this in practice:
*/

    dynamix::object o;
/*`
This object is empty. Mutation rules don't apply if there's no mutation. If,
however, the object had been created with a type template passed in its
constructor, then the rules would have been applied.
*/

    dynamix::mutate(o)
        .add<ofml_serialization>()
        .add<xml_serialization>()
        .add<wood_frame>();

/*`
Two rules are affected by this mutation. First it will implicitly add
`furniture` to the object, and second it will ignore the attempt to add
`ofml_serialization`. As a result the object will have `furniture`,
`xml_serialization` and `wood_frame`.
*/

    dynamix::mutate(o)
        .add<metal_frame>();

/*`
The mutually exclusive mixins will ensure that after this line the object won't
have the `wood_frame` mixin.

Having listed all built-in mutation rules, let's define a custom one.

Defining a custom rule is very easy. All you need to do is create a class
derived from `dynamix::mutation_rule` and override its pure virtual method
`apply_to`. The method has a single input-output parameter -- the mutation that
has been requested.

If you remember, we defined two mixins we haven't yet used -- `has_doors` and
`container`. We can safely say that a piece of furniture that has doors is
always also a container (The opposite is not true. Think racks and bookcases).
So it would be a good idea to add a mutation rule which adds the `container`
mixin if `has_doors` is being added, and removes `has_doors` if `container`
is being removed and the object has doors.
*/
    class container_rule : public dynamix::mutation_rule
    {
    public:
        virtual void apply_to(dynamix::object_type_mutation& mutation)
        {
            if(mutation.is_adding<has_doors>())
            {
                mutation.start_adding<container>();
            }

            if(mutation.is_removing<container>() && mutation.source_has<has_doors>())
            {
                mutation.start_removing<has_doors>();
            }
        }
    };

/*`
That's it. Now all we have to do is add our mutation rule and it will be
active.
*/
    dynamix::add_new_mutation_rule(new container_rule);

    dynamix::mutate(o)
        .add<has_doors>();

/*`
After this mutation our custom mutation rule has also added `container` to
the object.
*/

    dynamix::mutate(o)
        .remove<container>();

/*`
And after this line, thanks to our custom mutation rule, the object `o` will also have
its `has_doors` mixin removed.

To see all ways in which you can change a mutation from the mutation rule,
check out the documentation entry on `object_type_mutation`.

Lastly, there are two important pieces of information about mutation rules
that you need to know.

First, note that the library will be responsible for freeing the memory and
destroying the rules you've added. All you need to do is call
`add_new_mutation_rule` with a rule, allocated and constructed with `new`.

Second, you may have noticed that mutation rules can logically depend on
each other. You may ask yourselves what does the library do about that?
Does it do a topological sort of the rules? Say we add a mandatory /and/ a
deprecated rule about the same mixin. How does it handle dependency loops?

The answer is simple. It doesn't. The rules are applied once per mutation
in the order in which they were added. It is the responsibility of the user
to add them in some sensible order. Had the library provided some form
of rule sort, it would have needlessly overcomplicated the custom rule
definition, especially for cases in which you actually want to...
well, overrule a rule.

So, that's all there is to know about mutation rules.
*/

//]
    return 0;
}

//[tutorial_mutation_rules
//` %{tutorial_mutation_rules_A}
//` %{tutorial_mutation_rules_B}
//]
