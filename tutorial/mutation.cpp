// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>
#include <iostream>

using namespace std;

//[tutorial_mutation_A
/*`
For this tutorial let's begin by introducing some mixins that may be found in a
game: A root mixin, that should be present in all objects, and two that provide
a way to render the object on the screen:
*/

class game_object
{
    int _id;
    string name;
    // ... other common fields
};

class opengl_rendering {};

class directx_rendering {};

/*`
We won't concern ourselves with their concrete functionality, so we'll just
leave them with no messages.
*/
DYNAMIX_DEFINE_MIXIN(game_object, dynamix::none);
DYNAMIX_DEFINE_MIXIN(opengl_rendering, dynamix::none);
DYNAMIX_DEFINE_MIXIN(directx_rendering, dynamix::none);
//]

int main()
{
//[tutorial_mutation_B
/*`
You're probably familiar from the previous examples with the most basic way to
mutate an object, so let's use it to give this one a type.
*/
    dynamix::object obj1;
    dynamix::mutate(obj1)
        .add<game_object>()
        .add<opengl_rendering>();

/*`
...and then change it. Let's assume we're switching our rendering platform.
*/
    dynamix::mutate(obj1)
        .remove<opengl_rendering>()
        .add<directx_rendering>();

/*`
Using the `mutate` class is probably the most common way in which you'll mutate
objects in DynaMix. Yes, `mutate` is not a function but a class. It has
methods `remove` and `add`, and in its destructor it applies the actual
mutation.

A mutation is a relatively slow process so if the internal object type was being
changed on each call of `remove` or `add`, first the program would be needlessly
slowed down, and second the library would need to deal with various incomplete
types in its internal type registry.

So, if you want to add and remove mixins across several blocks or functions,
you may safely instantiate the `mutate` class or use its typedef
`single_object_mutator` that probably has a more appropriate name for cases like
this.
*/
    dynamix::single_object_mutator mutation(obj1);

    mutation.remove<directx_rendering>();
    // ...
    mutation.add<opengl_rendering>();

/*`
Here `obj1` hasn't been mutated yet. A type that has `game_object` and
`opengl_rendering` hasn't been instantiated internally. In order for this to
happen the `mutation` instance needs to be destroyed, or, to explicitly perform
the mutation, you may call `apply` like so:
*/

    mutation.apply();

/*`
Now `obj1` has been mutated, and `mutation` has been "cleared" -- returned to
the empty state it had right after its instantiation. This means we can reuse it
to perform other mutation on `obj1`. Say:
*/

    mutation.remove<game_object>();

/*`
Oops! We're removing the mixin that needs to be present in all objects. Not to
worry. You may "clear" a mutation without applying it, by calling `cancel`.
*/

    mutation.cancel();
/*`
Now the mutation is not performed, and its state is empty.

You may safely apply empty mutations to an object:
*/
    mutation.apply(); // no effect

/*`
Another way to mutate objects is by using a type template.

A type template gives a type to an object and, unlike
`mutate/single_object_mutator` it's not bound to a specific object instance.
Again unlike `mutate` it disregards all mixins within an object and applies its
internal type, hence it has no `remove` method. It implicitly "removes" all
mixins that are not among its own.

You can create a type template like so:
*/
    dynamix::object_type_template directx_rendering_template;
    directx_rendering_template
        .add<game_object>()
        .add<directx_rendering>()
        .create();

/*`
Again, similar to the case with `single_object_mutator`, you can spread these
calls among many blocks or functions.

Don't forget to call `create`. It is the method that creates the internal object
type. If you try to apply a type template that hasn't been created to an object,
a runtime error will be triggered.

To apply a type template to an object, you may pass it as a parameter to its
constructor.
*/

    dynamix::object obj2(directx_rendering_template);

/*`
Now `obj2` has the mixins `game_object` and `directx_rendering`.

Let's create a new type template.
*/

    dynamix::object_type_template opengl_rendering_template;
    opengl_rendering_template
        .add<game_object>()
        .add<opengl_rendering>()
        .create();

/*`
...to illustrate the other way of applying it to an object:
*/
    opengl_rendering_template.apply_to(obj2);

/*`
Applying this type template it the same object, was equivalent to
`mutate`-ing it, removing `directx_rendering` and adding `opengl_rendering`.

Now we have two objects -- `obj1` and `obj2` -- that have the same mixins.

Sometimes the case would be such that you have a big group of objects that have
the exact same type internally, and want them all to be mutated to have a
different type. Naturally you may `mutate` each of them one by one, and this
would be the appropriate (and only) way to mutate a group of objects that have
a /different/ type.

If the type is the same, however, you have a slightly faster alternative. The
same type mutator:
*/
    dynamix::same_type_mutator directx;
    directx
        .remove<opengl_rendering>()
        .add<directx_rendering>();

/*`
Unlike type templates, same type mutators don't need you to create them
explicitly with some method. The creation of the internal type and all
preparations are done when the mutation is applied to the first object.
*/
    directx.apply_to(obj1);
    directx.apply_to(obj2);

/*`
Remember that the only time you can afford to use a same type mutator, is when
/all/ objects that need to be mutated with it are composed of the same set of
mixins.
*/

//]
    return 0;
}

//[tutorial_mutation
//` %{tutorial_mutation_A}
//` %{tutorial_mutation_B}
//]
