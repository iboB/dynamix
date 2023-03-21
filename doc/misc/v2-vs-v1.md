# Differences Between DynaMix 1 and 2

This is a list of the most notable differences between v1 and v2:

* V2 has general mixin features. While there were plans to have mixin features other than messages in v1, they were never realized. V2's mixin features are general. Messages are just one possible type of feture. All mixin features of v2 in a type are put in the `ftable` (feature table) whereas the messages of v1 for a type were put in a `vtable`.
* V2 does not have hardcoded max numbers of features and mixins. This comes with a small performance hit, but it's just an additional if in each query. Uses of V1 demonstrated that this perf hit is acceptable.
* V2 supports multiple domains. There were plans to support multiple domains for v1, but they were never realized.
* V2 does not enforce mixin order by default. V1 guatantees that mixins will always appear in the same order in an object, regardless of the order in which they were added. By default v2 uses the order in which they were added. Not enforcing the order allows us to override features based on order, and not have to worry about bids or priority. V2 still can still support this behavior with `domain_settings::canonicalize_types`. Moreover with mutation rules other mixin orders can be enforced.
* V2 mixin are internal by default. V1 mixins were external - each mixin was allocated separately. V2 mixins by default are allocated in a single buffer per object. A V2 mixin can still be external if one of the following is true:
    * It has no move constructor
    * A `mixin_allocator` is assigned to this mixin
    * `mixin_info::force_external` is set to true
* V2 uses `std::pmr::polymorphic_allocator` for all allocations. V1 used to have dedicated allocators like `domain_allocator`, `object_allocator` and more. The only non-pmr allocator in V2 is the optional `mixin_allocator`
* V2 message definitions are a bit more verbose. V1 used to allow defining messages with a declare macro and a define macro which only used the message name. V2 messages, require an additional traits macro, and the definition requires the signature as well.

