// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#include <iostream>

using namespace std;

struct Point { int x, y; };
ostream& operator<<(ostream& o, const Point&) { return o; }

enum class Terrain
{
    Snow,
    Grass
};

class World
{
public:
    bool hasObstaclesBetween(const Point& a, const Point& b) const
    {
        return true;
    }

    Terrain terrainAt(const Point&) const
    {
        return Terrain::Snow;
    }
};

DYNAMIX_CONST_MESSAGE_0(const char*, name);
DYNAMIX_CONST_MESSAGE_0(const World&, world);
DYNAMIX_CONST_MESSAGE_0(Point, position);
DYNAMIX_CONST_MESSAGE_0(Point, decideTarget);

DYNAMIX_MESSAGE_1(void, moveTo, const Point&, target);
DYNAMIX_CONST_MESSAGE_1(bool, canMoveTo, const Point&, target);

class WalkingCreature
{
public:
    void moveTo(const Point& t) {
        cout << name(dm_this) << " walking to " << t << "\n";
    }
    bool canMoveTo(const Point& t) const {
        return !world(dm_this).hasObstaclesBetween(position(dm_this), t);
    }
};

DYNAMIX_DEFINE_MIXIN(WalkingCreature, moveTo_msg & canMoveTo_msg);

class FlyingCreature
{
public:
    void moveTo(const Point& t) {
        cout << name(dm_this) << " flying to " << t << "\n";
    }
    bool canMoveTo(const Point& t) const {
        return true;
    }
};

DYNAMIX_DEFINE_MIXIN(FlyingCreature, moveTo_msg & canMoveTo_msg);

class KeyboardControl
{
public:
    const char* name() const { return "hero"; }
    const World& world() const { static World w; return w; }
    Point position() const { return{ 1, 2 }; }
    Point decideTarget() const { return{ 1, 2 }; }
};

DYNAMIX_DEFINE_MIXIN(KeyboardControl, name_msg & world_msg & position_msg & decideTarget_msg);

class EnemyAI
{
public:
    const char* name() const { return "dragon"; }
    const World& world() const { static World w; return w; }
    Point position() const { return{ 1, 2 }; }
    Point decideTarget() const { return{ 1, 2 }; }
};

DYNAMIX_DEFINE_MIXIN(EnemyAI, name_msg & world_msg & position_msg & decideTarget_msg);

class FriendAI
{
public:
    const char* name() const { return "friendly dragon"; }
    const World& world() const { static World w; return w; }
    Point position() const { return{ 1, 2 }; }
    Point decideTarget() const { return{ 1, 2 }; }
};

DYNAMIX_DEFINE_MIXIN(FriendAI, name_msg & world_msg & position_msg & decideTarget_msg);

class AfraidOfSnow
{
public:
    bool canMoveTo(const Point& t) const {
        return world(dm_this).terrainAt(t) != Terrain::Snow;
    }
};

DYNAMIX_DEFINE_MIXIN(AfraidOfSnow, priority(1, canMoveTo_msg));

int main()
{
    vector<unique_ptr<dynamix::object>> objects;

    auto mainLoopIteration = [&objects]()
    {
        for (auto& obj : objects)
        {
            auto target = decideTarget(*obj);
            if (canMoveTo(*obj, target))
            {
                moveTo(*obj, target);
            }
        }
    };

    auto hero = new dynamix::object;
    dynamix::mutate(hero)
        .add<WalkingCreature>()
        .add<KeyboardControl>();
    objects.emplace_back(hero);

    auto dragon = new dynamix::object;
    dynamix::mutate(dragon)
        .add<FlyingCreature>()
        .add<EnemyAI>();
    objects.emplace_back(dragon);

    mainLoopIteration();

    dynamix::mutate(hero)
        .remove<WalkingCreature>()
        .add<FlyingCreature>();
    mainLoopIteration();

    dynamix::mutate(dragon)
        .remove<EnemyAI>()
        .add<FriendAI>();
    mainLoopIteration();

    dynamix::mutate(dragon)
        .add<AfraidOfSnow>();
    mainLoopIteration();

    dynamix::mutate(dragon)
        .remove<AfraidOfSnow>();
    mainLoopIteration();

    return 0;
}


DYNAMIX_DEFINE_MESSAGE(moveTo);
DYNAMIX_DEFINE_MESSAGE(canMoveTo);
DYNAMIX_DEFINE_MESSAGE(name);
DYNAMIX_DEFINE_MESSAGE(world);
DYNAMIX_DEFINE_MESSAGE(position);
DYNAMIX_DEFINE_MESSAGE(decideTarget);
