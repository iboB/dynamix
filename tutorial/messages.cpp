// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <iostream>
#include <string>
#include <sstream>
#include <dynamix/dynamix.hpp>

using namespace std;

//[tutorial_messages_A
/*`
For this tutorial we'll look at a simplified piece of code from an imaginary game.
First let's define the mixin classes that we're going to use.

There's a mixin that's a part from every object of our game. The one that gives
them a unique id. We'll also define a method, called `trace` that will display
information about the mixin in a stream.
*/

class has_id
{
public:
    void set_id(int id) { _id = id; }
    int id() const { return _id; }

    void trace(ostream& out) const;

private:
    int _id;
};

/*`
Next we'll define a class for an animated model. We could have other types of
models in the game, but for this tutorial there's no need to define anything
more.

The mixin offers us a way to set a mesh and two ways to set an animation. It has
a render method and, again the trace method, to display info about this mixin.
*/

class animated_model
{
public:
    void set_mesh(const string& mesh);

    void set_animation(const string& animation);
    void set_animation(int anim_id);

    void render() const;

    void trace(ostream& out) const;

private:
    string _mesh;
    string _animation;
};

/*`
Now we'll define three types of mixins that will give us artificial intelligence
logic for different occasions. They all share a method called `think` for the
AI, and the familiar trace method.
*/

class enemy_ai
{
public:
    void think();

    void trace(ostream& out) const;
};

class ally_ai
{
public:
    void think();

    void trace(ostream& out) const;
};

class stunned_ai
{
public:
    void think();

    void trace(ostream& out) const;
};

/*`
Now it's time to declare the messages our mixins will use. We have some methods
in our classes for which there won't be any messages, since those methods aren't
polymorphic. They're unique for their specific classes so it's absolutely
adequate to call them by `object.get<mixin>()->method(...)`.

So, let's start with the simplest case. The one we already used in the
[link dynamix.basic basic usage example].

The declaration syntax is the familiar macro `DYNAMIX_MESSAGE_|N|`, where
`|N|` stands for the number of arguments the message has. The macro's arguments
are coma separated: return value, message/method name, argument 1 type, argument
1 name, argument 2 type, argument 2 name, etc etc.

This simple case is covered by the messages `think` and `set_mesh` [footnote
Although set_mesh is a message that can be handled by a single class in our
example, in an actual product there would be other types of "model" mixins,
which would make it polymorphic. That's why we're making it a message instead
of a method to be called by `object.get<animated_model>()->set_mesh(somemesh)`]
*/

DYNAMIX_MESSAGE_0(void, think);
DYNAMIX_MESSAGE_1(void, set_mesh, const string&, mesh);

/*`
Now it may seem that `render` is also a pretty simple example of a message, but
there's a small difference. It's supposed to be handled by const methods. This
makes it a const message and as such it has a different declaration macro -- the
same as before but with `CONST` added to it:
*/

DYNAMIX_CONST_MESSAGE_0(void, render);

/*`
Lets see the `trace` method, that's present in all of our classes. If we declare
a message for it in the way we talked above, only of the mixins within an object
will be able to handle it. But when we `trace` an object's info, we obviously
would like to have the info for all of its mixins. For cases like this: where
more than one of the mixins in an object is supposed to handle a message,
DynaMix introduces /multicast/ messages. You declare those by adding
`MULTICAST` to the macro (before `MESSAGE` but after `CONST` if it's a const
one)
*/

DYNAMIX_CONST_MULTICAST_MESSAGE_1(void, trace, ostream&, out);

/*`
The last type of message there is meant for overloaded methods. For these we
need message overloads.

A message overload will require you to think of a special name, that's used to
refer to that message, different from the name of the method. Don't worry.
The stand-alone function that's generated for the message call itself will have
the appropriate name (the method's name).

The macro used for message overloads is the same as before with `OVERLOAD` at
the end. The other difference is that its first argument should be the custom
name for the message (followed by the type, method name, and method/message
arguments like before).

In our case `set_animation` has two overloads:
*/

DYNAMIX_MESSAGE_1_OVERLOAD(set_anim_by_name, void, set_animation, const string&, animation);
DYNAMIX_MESSAGE_1_OVERLOAD(set_anim_by_id, void, set_animation, int, anim_id);

/*`
As you might have guessed, any message could be defined as a message overload
and indeed in the case where there are no overloads `DYNAMIX_MESSAGE_N(ret,
message_name, ...)` will just expand to `DYNAMIX_MESSAGE_N_OVERLOAD(message_name,
ret, message_name, ...)`

So, now that we've declared all our messages it's time to define them.

The macro used for defining a message is always the same, regardless of the
message's constness, mechanism (multicast/unicast), or overload. It has a single
argument -- the message's name.
*/

DYNAMIX_DEFINE_MESSAGE(think);
DYNAMIX_DEFINE_MESSAGE(set_mesh);
DYNAMIX_DEFINE_MESSAGE(render);
DYNAMIX_DEFINE_MESSAGE(trace);

/*`
For the overloads we should use our custom name:
*/

DYNAMIX_DEFINE_MESSAGE(set_anim_by_name);
DYNAMIX_DEFINE_MESSAGE(set_anim_by_id);

/*`
Great! Now that we have our messages it's time to define the classes from above
as mixins.

Normally if our program is spread across several files, you should use
`DYNAMIX_DECLARE_MIXIN` to declare that those classes are mixins, but since our
program is in a single file, it can be omitted. All of its functionality is also
in `DYNAMIX_DEFINE_MIXIN`.

We met the `DYNAMIX_DEFINE_MIXIN` macro from the basic example. It has
two arguments -- the mixin/class name and its feature list. The feature list is
a ampersand separated list of symbols that represent the mixin's features. It can
contain many things, but for now we'll focus on messages -- the ones this mixin
is supposed to handle.

The special thing here is that in order to distinguish the stand-alone function
that's generated to make message calls from the message, the library defines a
special symbol for each message. This symbol is used in the mixin feature list
and when checking whether a mixin implements a message. The symbol is the
message name postfixed with `_msg`.

Let's define three of our simple mixins along with their feature (message)
lists:
*/

DYNAMIX_DEFINE_MIXIN(enemy_ai, think_msg & trace_msg);
DYNAMIX_DEFINE_MIXIN(ally_ai, think_msg & trace_msg);
DYNAMIX_DEFINE_MIXIN(animated_model,
    trace_msg & set_mesh_msg & set_anim_by_id_msg & set_anim_by_name_msg & render_msg);

/*`
The reason we left out `has_id` and `stunned_ai` is because we'd like to do
something special with their message lists.

[indexterm2 priority..example]

First, about `has_id`. What we'd like to do is display its info first, because
the object id is usually the first thing you need about an object. So in order
to achieve this, the notion of message priority is introduced. Each message in
a mixin gets a priority of 0 by default. For multicast messages, like `trace`, the
priority will affect the order in which they're executed. The higher priority
a multicast message has in a mixin, the earlier it will be executed. So if we
set the priority of `trace` in `has_id` to something greater than zero, we'll
have a guarantee that when the object info is displayed, its id will come first.
*/

DYNAMIX_DEFINE_MIXIN(has_id, priority(1, trace_msg));

/*`
For unicast messages the priority determines which of the potentially many mixin
candidates will handle the message. Again, mixins with higher priority for a
message are considered better candidates.

So if we set the priority of `think` in `stunned_ai` to something greater than
zero, then adding this mixin to an object that already has a think message
(like objects with `enemy_ai` or `ally_ai`), will hide it previous implementation
and override it with the one from `stunned_ai`. If we remove the mixin, the
previous implementation will be exposed and will resume handling the `think`
calls.

Also we'll consider `stunned_ai` as a relatively uninteresting mixin, and set
the priority of `trace` to -1, and make its info be displayed last (if at all
available)
*/

DYNAMIX_DEFINE_MIXIN(stunned_ai, priority(1, think_msg) & priority(-1, trace_msg));

//]

int main()
{
//[tutorial_messages_B
/*`
We're now ready to start using our mixins and messages in the simplified game.

Let's start by creating two objects - an enemy and an ally to the hypothetical
main character. We'll give them some irrelevant id-s and meshes.
*/

    dynamix::object enemy; // just an empty dynamix::object

    dynamix::mutate(enemy)
        .add<has_id>()
        .add<animated_model>()
        .add<enemy_ai>();

    enemy.get<has_id>()->set_id(1);
    set_mesh(enemy, "spider.mesh");

    trace(enemy, cout); // trace enemy data

    dynamix::object ally;

    dynamix::mutate(ally)
        .add<has_id>()
        .add<animated_model>()
        .add<ally_ai>();

    ally.get<has_id>()->set_id(5);
    set_mesh(ally, "dog.mesh");

    trace(ally, cout); // trace ally data

/*`
Both calls to `trace` from above will display info about the newly constructed
objects in the console.
*/

    think(enemy); // doing enemy stuff
    think(ally); // doing friendly stuff

    render(enemy); // drawing a hostile enemy
    render(ally); // drawing a friendly ally

/*`
Now lets try stunning our enemy. We'll just add the `stunned_ai` mixin and,
because of its special `think` priority, the calls to `think` from then on will
be handled by it.
*/

    dynamix::mutate(enemy).add<stunned_ai>();
    think(enemy); // don't do hostile stuff, because you're stunned
    render(enemy); // drawing a stunned enemy

/*`
Now let's remove the stun effect from our enemy, by simply removing the
`stunned_ai` mixin from the object. The handling of `think` by `enemy_ai` will
resume as before.
*/

    dynamix::mutate(enemy).remove<stunned_ai>();
    think(enemy); // again do hostile stuff
    render(enemy); // drawing a hostile enemy

/*`
And that concludes our tutorial on messages.
*/

//]

    return 0;
};

void has_id::trace(ostream& out) const
{
    out << "object with id: " << _id << endl;
}

void animated_model::set_mesh(const string& mesh)
{
    _mesh = mesh;
}

void animated_model::set_animation(const string& animation)
{
    _animation = animation;
}

void animated_model::set_animation(int anim_id)
{
    ostringstream sout;
    sout << anim_id;

    _animation = sout.str();
}

void animated_model::render() const
{
    cout << "rendering " << _mesh << " with anim " << _animation << endl;
}

void animated_model::trace(ostream& out) const
{
    out << "\twith animated model " << _mesh << endl;
}

void enemy_ai::think()
{
    set_animation(dm_this, "attack");
    cout << "kicking the player in the shin" << endl;
}

void enemy_ai::trace(ostream& out) const
{
    cout << "\twith enemy ai" << endl;
}

void ally_ai::think()
{
    set_animation(dm_this, "heal");
    cout << "putting a bandage on the players wounded shin" << endl;
}

void ally_ai::trace(ostream& out) const
{
    cout << "\twith ally ai" << endl;
}

void stunned_ai::think()
{
    set_animation(dm_this, "dizzy");
    cout << "cannot do anything because i'm stunned" << endl;
}

void stunned_ai::trace(ostream& out) const
{
    cout << "\tthat is stunned" << endl;
}


//[tutorial_messages
//` %{tutorial_messages_A}
//` %{tutorial_messages_B}
//]
