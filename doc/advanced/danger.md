# Dangerous Functionalities

Certain functionalities of the library are possible provided some constraints. They are deemed dangerous because the constraints cannot be enforced through code and wrapped in a safe API. Their use should be should be discouraged but in certain scenarios they can be helpful (and hopefully accompanied with at least a comment in the spirit of `// I know what I'm doing` ). 

## Accessing the object from a mixin's constructor or destructor

Is it safe to do something with `dm_this` from a mixin constructor or destructor? 

The library takes care provide a valid object to mixins while they're constructed or destroyed. So `dm_this` and `object_of(this)` will lead to a valid object pointer. This is a part of DynaMix's "contract". This will not change.

However *the type of the object is not reliable* at this point. As long as you don't touch the type, you're safe. Touching the type includes:

* Naturally, doing anything with the result of `object::get_type`
* Querying the object for mixins or features
* Accessing other mixins of the object
* Executing feature implementations (including calling messages)
* Mutating the object

Accessing the object without touching the type can certainly be useful in many scenarios and should not be a big no-no. Sadly the touching of the type cannot be safeguarded in any meaningful way. Moreover, going further into the rabit hole of borderline undefined behavior, touching the type in *"certain ways"* *may*, in some cases, be safe.

Here is an incomplete list of what would likely work, but doing anything from that list *should* be considered a big no-no and should only be done if nothing else works. 

Also note that anything after this line is not explicitly supported. Future versions of the library may make any of these unsafe again.

* When the object is not mutated, but copied or moved, touching the type may be safe. Touching mixins which are constructed (the ones before the current one) should also be safe.
* When the object is destroyed, accessing the type from a mixin destructor should be safe, but note that other mixins of the object may already be destroyed - the ones after the current one since the destruction order is the oposite of the construction order
* When the object is mutated the result of `object::get_type` in a mixin's constructor or destrutor would be the old object type - the one it's mutated from. This may be what one needs, who knows. Queries on the object will be equivalent to queries before the mutation took place.
* Accessing other mixins of the object which are *external* should be safe, as long as the above is taken into consideration: the type is the old type

## Mutating an object within a message

Is it safe to call `mutate(dm_this`? The answer is a definitive "maybe".

*TBD*
