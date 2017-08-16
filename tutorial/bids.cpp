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

class character
{
public:
    int get_health() const
    {
        return m_health;
    }

    void take_damage(int dmg)
    {
        m_health -= dmg;
    }

private:
    int m_health = 100;
};

DYNAMIX_CONST_MESSAGE_0(int, get_health);
DYNAMIX_MESSAGE_1(void, take_damage, int, dmg);

DYNAMIX_DEFINE_MIXIN(character, get_health_msg & take_damage_msg);

DYNAMIX_CONST_MULTICAST_MESSAGE_1(void, supply_rendering_data, vector<string>&, out_data);

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

class mesh
{
public:
    void supply_rendering_data(vector<string>& out_data) const
    {
        ostringstream sout;
        sout << "Mesh: " << m_mesh;
        out_data.push_back(sout.str());
    }

    string m_mesh;
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

class invisibility
{
public:
    void supply_rendering_data(vector<string>& out_data) const
    {
        out_data.push_back("A blur");
    }
};

DYNAMIX_DEFINE_MIXIN(invisibility, bid(1, supply_rendering_data_msg));

class stoneskin
{
public:
    void take_damage(int dmg)
    {
        DYNAMIX_CALL_NEXT_BIDDER(take_damage_msg, dmg / 2);
    }

    int get_health() const
    {
        return DYNAMIX_CALL_NEXT_BIDDER(get_health_msg) + 10;
    }

};

DYNAMIX_DEFINE_MIXIN(stoneskin, bid(1, get_health_msg) & bid(1, take_damage_msg));

int main()
{
    dynamix::object hero;

    dynamix::mutate(hero)
        .add<character>()
        .add<mesh>()
        .add<health_bar>();

    hero.get<mesh>()->m_mesh = "hero.mesh";

    render(hero);

    dynamix::mutate(hero)
        .add<invisibility>();

    render(hero);

    dynamix::mutate(hero)
        .remove<invisibility>();

    take_damage(hero, 20);
    render(hero);

    dynamix::mutate(hero)
        .add<stoneskin>();

    render(hero);

    take_damage(hero, 50);

    render(hero);

    return 0;
}

DYNAMIX_DEFINE_MESSAGE(get_health);
DYNAMIX_DEFINE_MESSAGE(take_damage);
DYNAMIX_DEFINE_MESSAGE(supply_rendering_data);
