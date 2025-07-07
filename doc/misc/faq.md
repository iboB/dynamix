# Frequently Asked Questions

## Why the C++ core? Why not C?

It's true that the core of the library is in C++ and the C interface is mostly composed of functions which wrap the C++ one. It's also true that some of the most basic data structures of the library are defined in C, and C++ wraps *them*. This is seemingly the worst of all worlds.

C++ has containers and it was relatively less effort to write simple algorithms making use of these. C++ has `pmr` which allows for an idiomatic and compatible allocator solution.

However these were not the deciding factors. I was prepared to invest more time and write the library's core entirely in C, and ditch the idiomatic allocators to get faster compilation and more portability. The deciding factor was exceptions. C has no exceptions and C++ exceptions generally cannot penetrate C functions. Now, it is true that most compilers support a way to allow C functions to be transparent to C++ exceptions, but this would require imposing special compilation settings on users, and, more importantly, there would be no way to properly clean up if an exception is thrown. C functions may be made to pass C++ exceptions, but they cannot catch them and do something about them.

I toyed with the idea of forbidding exceptions entirely but this would have made the life of C++ users much harder. C++ does throw exceptions on bad allocations. A lot of C++ software relies on exceptions. Exceptions are the only way to produce an error from a constructor. I am primarily a C++ programmer and I wanted first-class C++ support. Ditching exceptions was simply not an option.

Still I also want first-class C support. Having the core in C++ bars DynaMix from platforms which have no C++ compiler, but these are exceedingly rare. The functionalities which are most critical to performance (accessing mixins in objects, accessing features in a type) are written in C and there is no performance penalty from using them there. All library functionality should be available in C and everything which is possible in C++ should also be possible in C, though possibly with more verbose code (but this is normal for C).

## What's with all these macros?

Many people, upon seeing DynaMix for the first time, have expressed a concern with the seemingly excessive amount of macros the library's users are required to write.

First, there is no *requirement* to write macros. They are used to generate code, but the generated code can, of course, be written by hand. Macros are used where such manual code entry would involve a lot of careful synchronization and could easily lead to bugs without extra care.

The most important use case for the macros in DynaMix is global instantiation. Without them, users will be required to provide explicit entry points for their subsystems and dynamic libraries, where they will have to register mixins.

Still, ideas of how to reduce the macro count are welcome.

It is also possible (and probably part of the distant future of the library) to create an external tool that makes the user code a bit nicer. It would resemble [The Meta-Object Compiler of Qt](https://doc.qt.io/qt-6/moc.html), and similarly, would require a custom preprocessing step of the user's code.

Such a tool could theoretically solve other unpleasant problems, like the need to call `message(object)` instead of `object.message()` at the very least, and more...

Qt has shown that such an approach also has many opponents, as the code you write when you use it becomes effectively not-C++, but something that can be called a C++ dialect.
