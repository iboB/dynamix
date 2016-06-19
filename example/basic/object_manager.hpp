// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

class object_manager
{
public:
    ~object_manager();

    void create_objects();

    void change_rendering_sytem();

    std::vector<dynamix::object*>& objects()
    {
        return _objects;
    }

private:
    std::vector<dynamix::object*> _objects;
};
