// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

namespace dynamix
{

class object;

/**
 * \brief gets the object of a mixin
 *
 * \param[in] mixin_addr the address of the mixin
 *
 * \return A pointer to the object of the given mixin
 *
 * Returns the owning object of a given mixin.
 * \warning This function just makes a pointer offset and cast.
    It will work with any object that's been given to it
    without a warning or an error. Even, say `int*`. It is a
    source of potential bugs if you don't make sure that the
    input pointer is a mixin, that is a part of an object
 *
 * \par Example:
 * \code
 * mymixin* ptr = myobject->get<mymixin>();
 * object_of(ptr); // == myobject
 * \endcode
 *
 * \see #dm_this
 */
template <typename Mixin>
object* object_of(Mixin* mixin_addr)
{
    return *reinterpret_cast<object**>(reinterpret_cast<char*>(mixin_addr) - sizeof(object*));
}

/**
 * \copydoc object_of()
*/
template <typename Mixin>
const object* object_of(const Mixin* mixin_addr)
{
    return *reinterpret_cast<const object*const*>(reinterpret_cast<const char*>(mixin_addr) - sizeof(object*));
}

} // namespace dynamix
