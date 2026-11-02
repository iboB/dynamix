# DynaMix Documentation

> [!NOTE]
> This is unfortunately still in progress and being actively improved. Entries without links are not written yet.

## Getting Started

* [Overview](overview.md) - What is DynaMix and why might you want to use it?
* [Adding DynaMix to a Project](basics/adding.md) - Get up and running quickly
* [Hello World](basics/hello-world.md) - A walkthrough of a minimal DynaMix program
* [Core Concepts and Glossary](basics/glossary.md) - Learn the core concepts

## C++ Guide

* [Domains](cpp/domains.md)
* [Defining Mixins](cpp/mixins.md)
* [Messages](cpp/messages.md)
* [Objects](cpp/objects.md)
* [Mutations](cpp/mutations.md)
* Mutation Rules
* Type Classes
* Accessing the Object from a Mixin
* Error Handling

## C Guide

* The C Interface - How it differs from the C++ one

## Advanced Topics

* Allocators and Internal/External Mixins
* Multiple Domains, Shared Libraries, and Plugins
* Custom Mutation Rules
* Custom Features
* Mixins Without Macros
* Reflection and Introspection
* Hotswap
* [Dangerous Functionalities](advanced/danger.md)

## Working with DynaMix

* Performance
* Thread Safety
* Debugging
* Tips and Tricks
* Common Problems and Solutions

## Examples

* [Hello World](../example/hello-world/hello-world.cpp) - A minimal C++ program
* [Hello C World](../example/hello-c-world/hello-world.c) - A minimal C program
* [App and Plugins](../test/app-plugin/) - An application with a core library and plugins which extend and modify its objects

## Reference

*Coming soon*

## Miscellaneous

* [FAQ](misc/faq.md) - Frequently Asked Questions
* [Differences Between v1 and v2](misc/v2-vs-v1.md) - What changed between DynaMix v1 and v2
* [Migrating from v1](misc/migrating-from-v1.md) - How to migrate your code from DynaMix v1 to v2
* [Comparisons: DynaMix vs X](misc/dynamix-vs-x.md) - Comparisons with other solutions
* [Non-features](misc/non-features.md) - Things DynaMix deliberately does not do
* History and Inspiration

## Development

* Implementation Notes
* [Roadmap](roadmap.md) - What's coming next
