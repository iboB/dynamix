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

//[serialization_intro
/*`
Of course dealing with objects in a complex system means also having some way to
save and load them. Be it from a file, database, or through a network. We did
mention serialization before in our examples but we never gave an example of how
you can serialize `dynamix::object`-s.

The main issue would be how to convert the object's type information to a string
and then use this string to create an object with the same type information. As
for the concrete serialization of the data within the mixins, this example will
only offer a very na\u00EFve approach. We don't recommend that you use such an
approach for your mixins as it is not safe, and not backward compatible. We only
use it here because it's very easy to write and the focus of this example is on
the object type information.

Basically what we'll do here to save and load the data inside the mixins is
rely on two multicast messages &ndash; `save` and `load` &ndash; which will have a given
priority for each mixin. Thus ensuring the order of execution to be the same in
loading as it is in saving. Each save and load method of the mixin type will
assume it has the right data to save and load.

So, let's declare and define our messages.
*/
DYNAMIX_CONST_MULTICAST_MESSAGE_1(void, save, ostream&, out);
DYNAMIX_MULTICAST_MESSAGE_1(void, load, istream&, in);

DYNAMIX_DEFINE_MESSAGE(save);
DYNAMIX_DEFINE_MESSAGE(load);

/*`
Now let's define some simple mixins. For this example we'll assume we're writing
a company management system, which has a database of key individuals &ndash;
employees and clients.
*/

class person
{
public:
    void set_name(const string& name) { _name = name; }

    void save(ostream& out) const;
    void load(istream& in);

    static const int serialize_priority = 1;

private:
    string _name;
};
DYNAMIX_DEFINE_MIXIN(person,
    dynamix::priority(person::serialize_priority, save_msg)
    & dynamix::priority(person::serialize_priority, load_msg));

class employee
{
public:
    void set_position(const string& position) { _position = position; }

    void save(ostream& out) const;
    void load(istream& in);

    static const int serialize_priority = 2;

private:
    string _position;
};
DYNAMIX_DEFINE_MIXIN(employee,
    dynamix::priority(employee::serialize_priority, save_msg)
    & dynamix::priority(employee::serialize_priority, load_msg));

class client
{
public:
    void set_organization(const string& position) { _organization = position; }

    void save(ostream& out) const;
    void load(istream& in);

    static const int serialize_priority = 3;

private:
    string _organization;
};
DYNAMIX_DEFINE_MIXIN(client,
    dynamix::priority(client::serialize_priority, save_msg)
    & dynamix::priority(client::serialize_priority, load_msg));

/*`
Now let's declare the save and load functions for an object.

Because in this example they're stand-alone functions, like the messages, we
can't just name them save and load, because they'll clash with the message
functions defined by the message macros. So let's just name them `save_obj` and
`load_obj`.
*/
void save_obj(const dynamix::object& o, ostream& out);
void load_obj(dynamix::object& o, istream& in);
//]

int main()
{
//[serialization_use
/*`
Assuming those functions are written and work correctly, we could write some
code with them like this.

First we create some objects:
*/
    dynamix::object e;
    dynamix::mutate(e)
        .add<person>()
        .add<employee>();

    e.get<person>()->set_name("Alice Anderson");
    e.get<employee>()->set_position("Programmer");

    dynamix::object c;
    dynamix::mutate(c)
        .add<person>()
        .add<client>();

    c.get<person>()->set_name("Bob Behe");
    c.get<client>()->set_organization("Business Co");

/*`
Then we save them to some stream:
*/

    ostringstream out;
    save_obj(e, out);
    save_obj(c, out);

/*`
And finally use that stream to load those objects:
*/
    string data = out.str();

    istringstream in(data);

    dynamix::object obj1, obj2;
    load_obj(obj1, in); // loading Alice
    load_obj(obj2, in); // loading Bob
//]

    save_obj(obj1, cout);
    save_obj(obj2, cout);

    return 0;
}

void save_obj(const dynamix::object& obj, ostream& out)
{
//[serialization_save_obj
/*`
Now, let's see what we need to do to make the code from above work as
expected.

First let's write the save function.

To save the object we'll need to write the names of its mixins. You might
know that mixins also have id-s, but saving id-s is not a safe operation
as they are generated based on the global instantiation order. This means
that different programs with the same mixins (like a client or a server), or
even the same program after a recompilation, could end up generating
different id-s for the same mixins.

To get the names of the mixins in an object we could use
`object::get_mixin_names` and it's perfectly fine, but in order to make this
example a bit more interesting, let's dive a bit into the library's internal
structure.

If you've read the implementation notes or the debugging tutorial, you'll
know that an object has a type information member which contains the mixin
composition of the object in a `std::vector` called `_compact_mixins`. We'll
use this vector to save the mixin names.
*/

    size_t num_mixins = obj._type_info->_compact_mixins.size();
    out << num_mixins << endl; // write the size
    for(size_t i=0; i<num_mixins; ++i)
    {
        // write each name
        out << obj._type_info->_compact_mixins[i]->name << endl;
    }

/*`
After we've stored the object type information, we can now save the data
within its mixins via the `save` message from above.
*/
    save(obj, out);
//]
}

void load_obj(dynamix::object& obj, istream& in)
{
//[serialization_load_obj
/*`
That was it. Now let's move to the code of the `load_obj` function.

First we need to get the number of mixins we're loading. Let's do it in this
simple fashion:
*/
    string line;
    getline(in, line);

    size_t num_mixins = atoi(line.c_str());

/*`
Now we'll create an `object_type_template` which we'll use to store the
loaded type and give it to a new object. The type template class (as all
other mutator classes) has the method `add`. Besides the way you're used to
calling it &ndash; `add<mixin>()` &ndash; you can also call it with a `const char*`
argument, which will be interpreted as a mixin name.

When being called like this, it will return `bool`. True if a mixin of this
name was found, and false if it wasn't. Note that this true or false value
does not give you the information on whether the mixin will be added or
removed from the object, but only a mixin of name exists in the domain. As
you might remember the mutation rules (if such are added) will determine
whether the mixin is actually added an removed.
*/
    dynamix::object_type_template tmpl;
    for(size_t i=0; i<num_mixins; ++i)
    {
        getline(in, line);
        tmpl.add(line.c_str());
    }
    tmpl.create();

/*`
Now what's left is to apply the template to the object:
*/
    tmpl.apply_to(obj);

/*`
The last thing we need to do when loading an object is to load the data
within its mixins. The object is created with the appropriate mixins, so
let's call the `load` multicast message.
*/
    load(obj, in);

/*`
It should load the data correctly because the order of the save and load
execution is the same &ndash; determined by their priority.
*/
//]
}


void person::save(ostream& out) const
{
    out << _name << endl;
}

void person::load(istream& in)
{
    getline(in, _name);
}

void employee::save(ostream& out) const
{
    out << _position << endl;
}

void employee::load(istream& in)
{
    getline(in, _position);
}

void client::save(ostream& out) const
{
    out << _organization << endl;
}

void client::load(istream& in)
{
    getline(in, _organization);
}

//[tutorial_serialization
//` %{serialization_intro}
//` %{serialization_use}
//` %{serialization_save_obj}
//` %{serialization_load_obj}
//]
