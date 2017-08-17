// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <dynamix/dynamix.hpp>

using namespace std;

//[tutorial_bids_A
/*`
After we covered the basic features of messages: unicasts, multicasts,
priorities, overloads, and default implementations, let's now delve a bit deeper.
Let's focus on message bids.

For this tutorial let's suppose we're writing an RPG game. Let's define a
character mixin and some messages for it:
*/

class character
{
public:
    int get_health() const
    {
        return _health;
    }

    void take_damage(int dmg)
    {
        _health -= dmg;
    }

    // ...

private:
    int _health = 100;
};

DYNAMIX_CONST_MESSAGE_0(int, get_health);
DYNAMIX_MESSAGE_1(void, take_damage, int, dmg);

DYNAMIX_DEFINE_MIXIN(character, get_health_msg & take_damage_msg);

/*`
In this game the different objects would need to be rendered on the screen in
some way. For this, let's define functionality to do so.

Potentially multiple mixins in our objects would need some graphical
visualization. For this our rendering design will have the multicast message
`supply_rendering_data` which will fill an output list with the graphics
for each mixin which implements it:
*/

DYNAMIX_CONST_MULTICAST_MESSAGE_1(void, supply_rendering_data, vector<string>&, out_data);

/*`
For simplicity in our example we'll just use `std::string` as "rendering" data.

Now we'll define a `render` function which takes an objects, calls the `supply_rendering_data`
message and prints out the contents of string vector. In an real-world scenario
of this sort, of course it would have some way of supplying the result to a
rendering subsystem.
*/

void render(const dynamix::object& obj)
{
    vector<string> data;
    supply_rendering_data(obj, data);

    cout << "Rendering:\n";
    for (auto& elem : data)
    {
        cout << '\t' << elem << '\n';
    }
    cout << endl;
}

/*`
The idealized rendering mixins for our example are `mesh` and `health_bar`.
Meshes will represent how our object is visualized as a part of the game world,
while health bars will visualize the health of a character (if the object is
such) much like many RPG-s and strategy games do.

Getting the health for the object happens through the previously defined
`get_health` message. This is a polymophic message call, because not only
characters can have health in this game. Other objects might have it as well
(say destructible crates or obstacles).
*/

class mesh
{
public:
    void supply_rendering_data(vector<string>& out_data) const
    {
        ostringstream sout;
        sout << "Mesh: " << _mesh;
        out_data.push_back(sout.str());
    }

    string _mesh; // just use std::string for simplicity
};

DYNAMIX_DEFINE_MIXIN(mesh, supply_rendering_data_msg);

class health_bar
{
public:
    void supply_rendering_data(vector<string>& out_data) const
    {
        ostringstream sout;
        sout << "Health: " << get_health(dm_this);
        out_data.push_back(sout.str());
    }
};

DYNAMIX_DEFINE_MIXIN(health_bar, supply_rendering_data_msg);

/*`
Now, suppose that in our game we want some way of having invisible objects.
Ar first this might seem like a straight forward case. We can just creata a
mixin called `invisibility` which implements `supply_rendering_data` by adding
a blur (or nothing) to the output list. Like so:
*/

class invisibility
{
public:
    void supply_rendering_data(vector<string>& out_data) const
    {
        out_data.push_back("A blur"); // think StarCraft :)
    }
};

/*`
However `supply_rendering_data` is a multicast message. If we don't do
anything else, adding this mixin to an object will result in the output list
being filled with all existing parts *plus* a blur (or indeed nothing). Had
`supply_rendering_data` been a unicast message, then we could've added a bigger
priority to it so it overrides the original, but priority doesn't help us to
override multicasts. It just determines the order.

There are of course many solutions to our problem using what you've learned
so far (for example `invisibility::supply_rendering_data` can be last and used
to clear the ouput list, or some kind of multicast result combinator can be
used which breaks the mutlicast chain), but the cleanest and indeed most
optimal solution is to use bids to override the mulcitast like so:
*/

DYNAMIX_DEFINE_MIXIN(invisibility, bid(1, supply_rendering_data_msg));

/*`
Bids are similar to message priorities. For multicasts the priority determines
the order of execution when the message is called. The bid determines which
messages will be executed. They will be the ones with the highest bid (or top
bidders).

So in the example from above, since `invisibility` bids 1 for
`supply_rendering_data`, which is higher than the default zero, if we were to
add it to an object, it would override the `supply_rendering_data` message
(unless of course some mixins with an even higher bid are in there).

Of course, since this is a multicast, if we add other mixins which implement
`supply_rendering_data` with the same bid, 1, their implentations will also
be executed along with the one from `invisibility`.
*/
//]

//[tutorial_bids_C
/*`
We saw how message bids can help us override multicast messages but what about
unicasts. Is there a point to bids for them?

Yes there is but before we explain, let's continue with a motivating example.

Let's imagine that in our game we want a stoneskin effect. Stoneskin will cut
all damage taken by an object in half and let's also (admittedly pointlessly)
add the requirement that the stoneskin effect will add 10 more health points to
the object.

It obvious that the `stoneskin` mixin would need to override `take_damage` and
`get_health` to do so. So let's define our class:
*/

class stoneskin
{
public:
    void take_damage(int dmg);
    int get_health() const;
};

/*`
We now have a problem. How do we implement these functions? We do need some way
of transfering the newly calculated damage, or get the exisitng health. While
we could write something like `dm_this->get<character>()->take_damage(dmg/2);`,
we did mention that not only characters have health. Writing non-polymorphic
code such as this won't work if we add stoneskin to a destructible object.

Unicast bids will help us in this case.

Superifically bids for unicasts work like finer grain priorities. If an object
has mixins which implement the same unicast message with the same priority, the
implementation with the highest bid will be executed (note the priority is the
primary sort key in this case. So if one mixin implements a message with
priority 10 and bid 1, and another with priority 1 and bid 1000, the first one's
implenentation will be executed because it has the highest priority).

However when setting bids to unicasts, all bidders from the top priority will
be available in an object which implements a message. This allows us to call a
lower bidder from a higher one.

Think of this as calling the superclass's virtual method from the one that
overrides it in a sublass.

If you override a unicast message by adding a mixin to an object which already
implements it, when only priorities are involved, the overriden implementation
is inaccessible and lost until we remove the mixin. However bids allow us to
call the DYNAMIX_CALL_NEXT_BIDDER macro from a message implementation to
call a lower bidder with the same priority which we have overriden.
*/

// implement the messages with higher bids
DYNAMIX_DEFINE_MIXIN(stoneskin, bid(1, get_health_msg) & bid(1, take_damage_msg));

// call next bidders modifying values by the stoneskin effect
void stoneskin::take_damage(int dmg)
{
    DYNAMIX_CALL_NEXT_BIDDER(take_damage_msg, dmg / 2);
}

int stoneskin::get_health() const
{
    return DYNAMIX_CALL_NEXT_BIDDER(get_health_msg) + 10;
}

/*`
As you can see to call the next bidder, you need to supply the message tag as
an argument. Otherwise the macro behaves exacly like the underlying function:
it has the same arguments and the same return value.

The need to supply the message tag helps us to also call next bidders from
methods which don't necessarily implement the message in question.
*/
//]

int main()
{
//[tutorial_bids_B
/*`
Let's see our resulting code in action:
*/
    // create a "hero" object and add our mixins to it
    dynamix::object hero;

    dynamix::mutate(hero)
        .add<character>()
        .add<mesh>()
        .add<health_bar>();

    // set some "mesh"
    hero.get<mesh>()->_mesh = "hero.mesh";

    // render a mesh and a health bar
    render(hero);

    // add invisibility
    // it will override supply_rendering_data from mesh and health_bar
    dynamix::mutate(hero)
        .add<invisibility>();

    // render a blur
    render(hero);
//]

    dynamix::mutate(hero)
        .remove<invisibility>();

//[tutorial_bids_D
/*`
Now let's use our unicast bids and calling of next bidders:
*/
    // hit the hero with 20 damage
    take_damage(hero, 20);

    // rendering hero with health 80 (from initial 100)
    render(hero);

    dynamix::mutate(hero)
        .add<stoneskin>();

    // with stoneskin the health is +10
    // so now we render a hero with health 90
    render(hero);

    // hit the hero with 50 damage
    // stoneskin will transfer 25
    take_damage(hero, 50);

    // render hero with 80 + 10 - 25 = 65 health
    render(hero);
//]

    return 0;
}

DYNAMIX_DEFINE_MESSAGE(get_health);
DYNAMIX_DEFINE_MESSAGE(take_damage);
DYNAMIX_DEFINE_MESSAGE(supply_rendering_data);

//[tutorial_bids
//` %{tutorial_bids_A}
//` %{tutorial_bids_B}
//` %{tutorial_bids_C}
//` %{tutorial_bids_D}
//]
