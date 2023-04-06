# Non-features

This is a list of potential new features and improvements which have been considered and it was decided against them.

## Internal messages

An argument can be made that internal messages can be helpful in certain scenarios. The most common example here would be `on_mutated` - a multicast used to notify all mixins in an object that a mutation has happened and they could update potential internal relations that they have. `on_destroying` is another example called when the object is being destroyed. 

This would complicate the library, mixin infos, and object construction too much. Moreover we cannot possibly envision all service messages that one could need and going down the road of supporting this is not a good option.

Instead this should be solved on a per-domain basis, by extending mutations, objects and mixins as needed. For example `on_mutated` can be part of a domain where objects should not be mutaded through `mutate` but instead through a custom function like `notifying_mutate` which calls the message. 

If a use case for an internal message is found, which cannot be implemented through extending the library, but can by modifying it internally, this non-feature should be reconsidered.

## Object mutexes

Object mutexes which would allow say mutating objects in a thread while messages are called in another can be added by extending the library. There is even a demo planned to showcase such extensions. This will not be an internal feature.
