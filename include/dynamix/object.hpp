// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "global.hpp"
#include "mixin_type_info.hpp"

namespace dynamix
{

namespace internal
{
    class object_type_info;
    class mixin_data_in_object;
    struct message_t;
    struct message_feature_tag;
}

class object_type_template;

/// The main object class.
class DYNAMIX_API object
{
public:
    /// Constructs an empty object - no mixins.
    object();

    /// Constructs an object from a specific type template.
    explicit object(const object_type_template& type_template);

    ~object();

    /// Move constructor from an existing object
    object(object&& o);

    /// Move assignment
    object& operator=(object&& o);

#if DYNAMIX_OBJECT_IMPLICIT_COPY
    /// Copy constructor
    object(const object& o);

    /// Assignment
    /// Will also change the type of the target to the source type.
    /// Will call assignment operators for mixins that exist in both.
    /// Will copy-construct new mixins for this.
    /// Will destroy mixins that don't exist in source.
    /// It will not, however, match asssignment operators for different mixin types which have such defined between them,
    object& operator=(const object& o);
#else
    object(const object& o) = delete;
    object& operator=(const object& o) = delete;
#endif

    /// Explicit copy via move assignment
    /// This function is not recommended as it will slice the type if you have a class inherited from object
    object copy() const;

    /// Explicit assignment from existing object.
    /// Will also change the type of the target to the source type.
    /// Will call assignment operators for mixins that exist in both.
    /// Will copy-construct new mixins for this.
    /// Will destroy mixins that don't exist in source.
    /// It will not, however, match asssignment operators for different mixin types which have such defined between them,
    void copy_from(const object& o);

    /// Assignment of mixins that exist in both objects.
    /// Will not change the type of the target.
    /// Will call assignment operators for mixins that exist in both objects.
    /// It will not, however, match asssignment operators for different mixin types which have such defined between them,
    void copy_matching_from(const object& o);

    /// Checks whether all of the object's mixins have copy-constructors and assignment operators.
    /// Returns false if either is missing from at least one of its mixins
    /// (note that there might be cases where copy_from or copy_matching_from won't throw
    /// even though this function returns false).
    bool copyable() const;

    /////////////////////////////////////////////////////////////////
    // mixin info

    /// Checks if the object has a specific mixin.
    template <typename Mixin>
    bool has() const
    {
        const internal::mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        // intentionally disregarding the actual info
        return internal_has_mixin(info.id);
    }

    /// Gets a specific mixin from the object. Returns nullptr if the mixin
    /// isn't available.
    template <typename Mixin>
    Mixin* get()
    {
        const internal::mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        // intentionally disregarding the actual info
        return reinterpret_cast<Mixin*>(internal_get_mixin(info.id));
    }

    /// Gets a specific mixin from the object. Returns nullptr if the mixin
    /// isn't available.
    template <typename Mixin>
    const Mixin* get() const
    {
        const internal::mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        // intentionally disregarding the actual info
        return reinterpret_cast<const Mixin*>(internal_get_mixin(info.id));
    }

    /// Checks if the object has a specific mixin by id.
    bool has(mixin_id id) const;

    /// Gets a specific mixin by id from the object. Returns nullptr if the mixin
    /// isn't available. It is the user's responsibility to cast the returned
    /// value to the appropriate type
    void* get(mixin_id id);

    /// Gets a specific mixin by id from the object. Returns nullptr if the mixin
    /// isn't available. It is the user's responsibility to cast the returned
    /// value to the appropriate type
    const void* get(mixin_id id) const;
    /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // feature info

    /// Checks if the mixin implements a feature.
    template <typename Feature>
    bool implements(const Feature*) const
    {
        const Feature& f = static_cast<const Feature&>(_dynamix_get_mixin_feature_fast(static_cast<Feature*>(nullptr)));
        DYNAMIX_ASSERT(f.id != INVALID_FEATURE_ID);
        // intentionally disregarding the actual feature,
        // because of potential multiple implementations
        return internal_implements(f.id, typename Feature::feature_tag());
    }

    /// Returns the number of mixins in the object which implement a feature.
    template <typename Feature>
    size_t num_implementers(const Feature*) const
    {
        const Feature& f = static_cast<const Feature&>(_dynamix_get_mixin_feature_fast(static_cast<Feature*>(nullptr)));
        DYNAMIX_ASSERT(f.id != INVALID_FEATURE_ID);
        // intentionally disregarding the actual feature,
        // because of potential multiple implementations
        // the actual feature will be gotten from the feature registry in the domain
        return internal_num_implementers(f.id, typename Feature::feature_tag());
    }
    /////////////////////////////////////////////////////////////////

    /// Destroys all mixins within an object and resets its type info
    // (sets null type info)
    void clear();

    /// Returns true if the object is empty - has no mixins
    bool empty() const;

    /////////////////////////////////////////////////////////////////
    // logging and diagnostics

    /// Adds the names of the messages implemented by the object to the vector
    void get_message_names(std::vector<const char*>& out_message_names) const;

    /// Adds the names of the object's mixins to the vector
    void get_mixin_names(std::vector<const char*>& out_mixin_names) const;

    /////////////////////////////////////////////////////////////////

_dynamix_internal:
    void* internal_get_mixin(mixin_id id);
    const void* internal_get_mixin(mixin_id id) const;
    bool internal_has_mixin(mixin_id id) const;

    // reorganizes the mixins for the new type
    // if manage_mixins is true the object will destroy all mixins removed and construct all new ones
    // we have the manage_mixins argument because its a lot faster to manage from the outside
    // if we know exactly what's added and removed
    void change_type(const internal::object_type_info* new_type, bool manage_mixins/* = true*/);

    // performs the move from one object source to this
    // can only be performed on empty objects
    void usurp(object&& o);

    // constructs mixin with optional source to copy from
    // will return false if source is provided but no copy constructor exists
    bool construct_mixin(mixin_id id, const void* source);
    void destroy_mixin(mixin_id id);

    const internal::object_type_info* _type_info;

    // each element of this array points to a buffer which cointains a pointer to
    // this - the object and then the mixin
    // thus each mixin can get its own object
    internal::mixin_data_in_object* _mixin_data;

    bool internal_implements(feature_id id, const internal::message_feature_tag&) const
    {
        return implements_message(id);
    }

    bool implements_message(feature_id id) const;

    size_t internal_num_implementers(feature_id id, const internal::message_feature_tag&) const
    {
        return message_num_implementers(id);
    }

    size_t message_num_implementers(feature_id id) const;

    // virtual mixin for default message implementation
    // used only so as to not have a null pointer cast to the appropriate type for default implementations
    // which could be treated as an error in some debuggers
    struct default_impl_virtual_mixin { void* unused; };
    struct default_impl_virtual_mixin_data_in_object
    {
        object* obj;
        default_impl_virtual_mixin mixin;
    };
    default_impl_virtual_mixin_data_in_object _default_impl_virtual_mixin_data;
};

} // namespace dynamix
