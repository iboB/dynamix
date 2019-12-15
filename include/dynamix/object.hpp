// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "config.hpp"
#include "internal/assert.hpp"
#include "mixin_type_info.hpp"

namespace dynamix
{

namespace internal
{
class object_type_info;
class mixin_data_in_object;
struct message_t;
struct message_feature_tag;
} // namespace internal

class object_type_template;
class object_allocator;

/// The main object class.
class DYNAMIX_API object
{
public:
    /// Constructs an empty object - no mixins.
    object() noexcept;

    /// Constructs an object with an object allocator
    explicit object(object_allocator* allocator);
    /// Constructs an object from a specific type template.
    explicit object(const object_type_template& type_template, object_allocator* allocator = nullptr);

    ~object();

    /// Move constructor from an existing object
    object(object&& o) noexcept;

    /// Move assignment
    object& operator=(object&& o) noexcept;

#if DYNAMIX_OBJECT_IMPLICIT_COPY
    /// Copy constructor
    object(const object& o);

    /// Assignment
    /// Will also change the type of the target to the source type.
    /// Will call assignment operators for mixins that exist in both.
    /// Will copy-construct new mixins for this.
    /// Will destroy mixins that don't exist in source.
    /// It will not, however, match asssignment operators for different mixin types
    /// which have such defined between them.
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
    /// It will not, however, match asssignment operators for different mixin types
    /// which have such defined between them.
    void copy_from(const object& o);

    /// Assignment of mixins that exist in both objects.
    /// Will not change the type of the target.
    /// Will call assignment operators for mixins that exist in both objects.
    /// It will not, however, match asssignment operators for different mixin types
    /// which have such defined between them.
    void copy_matching_from(const object& o);

    /// Checks whether all of the object's mixins have copy-constructors and assignment operators.
    /// Returns false if either is missing from at least one of its mixins
    /// (note that there might be cases where copy_from or copy_matching_from won't throw
    /// even though this function returns false).
    bool copyable() const noexcept;

    /// Move-assignment of mixins that exist in both objects.
    /// Will call move-assignment operators of the mixins.
    /// Will not change the type of the target or the source, but will leave mathing mixins in the source
    /// in a moved-out-from state.
    void move_matching_from(object& o);

    /////////////////////////////////////////////////////////////////
    // mixin info

    /// Checks if the object has a specific mixin.
    template <typename Mixin>
    bool has() const noexcept
    {
        const mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        return internal_has_mixin(info.id);
    }

    /// Gets a specific mixin from the object. Returns nullptr if the mixin
    /// isn't available.
    template <typename Mixin>
    Mixin* get() noexcept
    {
        const mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        return reinterpret_cast<Mixin*>(internal_get_mixin(info.id));
    }

    /// Gets a specific mixin from the object. Returns nullptr if the mixin
    /// isn't available.
    template <typename Mixin>
    const Mixin* get() const noexcept
    {
        const mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        return reinterpret_cast<const Mixin*>(internal_get_mixin(info.id));
    }

    /// Checks if the object has a specific mixin by id.
    bool has(mixin_id id) const noexcept;

    /// Checks if the object has a specific mixin by mixin name (name of the class or
    /// manual name provided by the `mixin_name` feature).
    bool has(const char* mixin_name) const noexcept;

    /// Gets a specific mixin by id from the object. Returns nullptr if the mixin
    /// isn't available. It is the user's responsibility to cast the returned
    /// value to the appropriate type.
    void* get(mixin_id id) noexcept;

    /// Gets a specific mixin by id from the object. Returns nullptr if the mixin
    /// isn't available. It is the user's responsibility to cast the returned
    /// value to the appropriate type.
    const void* get(mixin_id id) const noexcept;

    /// Gets a specific mixin by mixin name from the object. Returns nullptr if the mixin
    /// isn't available. It is the user's responsibility to cast the returned
    /// value to the appropriate type.
    ///
    /// The mixin name is the name of the actual mixin class or a
    /// manual name provided by the `mixin_name` feature.
    void* get(const char* mixin_name) noexcept;

    /// Gets a specific mixin by mixin name from the object. Returns nullptr if the mixin
    /// isn't available. It is the user's responsibility to cast the returned
    /// value to the appropriate type.
    ///
    /// The mixin name is the name of the actual mixin class or a
    /// manual name provided by the `mixin_name` feature.
    const void* get(const char* mixin_name) const noexcept;
    /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // feature info

    /// Checks if the object implements a feature.
    template <typename Feature>
    bool implements(const Feature*) const noexcept
    {
        const Feature& f = static_cast<const Feature&>(_dynamix_get_mixin_feature_fast(static_cast<Feature*>(nullptr)));
        I_DYNAMIX_ASSERT(f.id != INVALID_FEATURE_ID);
        // intentionally disregarding the actual feature,
        // because of potential multiple implementations
        return internal_implements(f.id, typename Feature::feature_tag());
    }

    /// Checks if the object implements a feature by a mixin.
    /// Note that on `false` the object might still implement the feature but with a default implementation.
    template <typename Feature>
    bool implements_by_mixin(const Feature*) const noexcept
    {
        const Feature& f = static_cast<const Feature&>(_dynamix_get_mixin_feature_fast(static_cast<Feature*>(nullptr)));
        I_DYNAMIX_ASSERT(f.id != INVALID_FEATURE_ID);
        // intentionally disregarding the actual feature,
        // because of potential multiple implementations
        return internal_implements_by_mixin(f.id, typename Feature::feature_tag());
    }

    /// Checks if the object implements a feature with a default implementation
    /// (`false` means that it either does not implement it at all, or it's implemented by a mixin)
    template <typename Feature>
    bool implements_with_default(const Feature*) const noexcept
    {
        const Feature& f = static_cast<const Feature&>(_dynamix_get_mixin_feature_fast(static_cast<Feature*>(nullptr)));
        I_DYNAMIX_ASSERT(f.id != INVALID_FEATURE_ID);
        // intentionally disregarding the actual feature,
        // because of potential multiple implementations
        return internal_implements(f.id, typename Feature::feature_tag()) &&
               !internal_implements_by_mixin(f.id, typename Feature::feature_tag());
    }

    /// Returns the number of mixins in the object which implement a feature.
    template <typename Feature>
    size_t num_implementers(const Feature*) const noexcept
    {
        const Feature& f = static_cast<const Feature&>(_dynamix_get_mixin_feature_fast(static_cast<Feature*>(nullptr)));
        I_DYNAMIX_ASSERT(f.id != INVALID_FEATURE_ID);
        // intentionally disregarding the actual feature,
        // because of potential multiple implementations
        // the actual feature will be gotten from the feature registry in the domain
        return internal_num_implementers(f.id, typename Feature::feature_tag());
    }
    /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // memory and mixin management

    /// Destroys all mixins within an object and resets its type info
    // (sets null type info)
    void clear() noexcept;

    /// Returns true if the object is empty - has no mixins
    bool empty() const noexcept;

    /// Returns the allcator associated with this object (may be `nullptr`)
    object_allocator* allocator() const { return _allocator; }

#if DYNAMIX_OBJECT_REPLACE_MIXIN
    /// Moves a mixin to the designated buffer, by invocating its move constructor.
    /// Throws an exception if the mixin is not movable.
    /// Returns the old mixin buffer and offset or {nullptr, 0} if the object doesn't have such mixin
    /// The library never calls this function internally. Unless the user calls it, an object's mixins will always
    /// have the same addresses
    std::pair<char*, size_t> move_mixin(mixin_id id, char* buffer, size_t mixin_offset);

    /// Replaces a mixin's buffer with another. Returns the old buffer and offset.
    /// WARNING: if the Mixin is not part of the object, this function will crash!
    /// Will not touch the new buffer. It's the user's responsibility to set the appropriate
    /// object inside.
    /// The library never calls this function internally. Unless the user calls it, an object's mixins will always
    /// have the same addresses
    std::pair<char*, size_t> hard_replace_mixin(mixin_id id, char* buffer, size_t mixin_offset) noexcept;

    /// Allocates buffers for all mixins and deallocates the old ones. Suitable to call
    /// from object allocators which keep a single mixin buffer per object. The order is
    /// for each mixin: { allocate new; deallocate old; }
    /// The library never calls this function internally. Unless the user calls it, an object's mixins will always
    /// have the same addresses
    void reallocate_mixins();
#endif
    /////////////////////////////////////////////////////////////////

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
    // destroys all mixins removed and construct all new ones
    void change_type(const internal::object_type_info* new_type);

    enum class change_type_from_result
    {
        success,
        bad_assign,
        bad_copy_construct
    };

    // changes the type and optionally copies mixins from the source
    // if source is null will default_construct new mixins and destroy old ones
    // if source is not null it
    // copy-assigns from source the ones we already have
    // copy-constructs from source ones we don't have
    // destroys mixins which are not in new_type
    // this function is only valid if the source mixins are null or from the same type info as new_type
    change_type_from_result change_type_from(const internal::object_type_info* new_type, const internal::mixin_data_in_object* source);

    // performs the move from one object source to this
    // can only be performed on empty objects
    void usurp(object&& o) noexcept;

    // allocates memory and
    // constructs mixin with optional source to copy from
    // will return false if source is provided but no copy constructor exists
    bool make_mixin(const mixin_type_info& mixin_info, const void* source);

    // destroys mixin and deallocates memory
    void delete_mixin(const mixin_type_info& mixin_info);

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

    bool internal_implements_by_mixin(feature_id id, const internal::message_feature_tag&) const
    {
        return implements_message_by_mixin(id);
    }

    bool implements_message_by_mixin(feature_id id) const;

    size_t internal_num_implementers(feature_id id, const internal::message_feature_tag&) const
    {
        return message_num_implementers(id);
    }

    size_t message_num_implementers(feature_id id) const;

    // optional allocator for this object
    object_allocator* _allocator = nullptr;

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
