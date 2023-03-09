var NAVTREE =
[
  [ "DynaMix", "index.html", [
    [ "Introduction", "index.html", [
      [ "Abstract ", "index.html#abstract", [
        [ "Other meanings of \"mixin\" in C++ ", "index.html#other_mixin", null ],
        [ "Other meanings of \"mixin\" in other languages ", "index.html#other_lang", null ],
        [ "Basic look and feel of the code with DynaMix ", "index.html#look_feel", null ]
      ] ],
      [ "List of features ", "index.html#features", null ],
      [ "When (and when not) to use DynaMix ", "index.html#when_to_use", null ],
      [ "Comparisons ", "index.html#comparisons", [
        [ "Comparison with multiple inheritance ", "index.html#multiple_inh", null ],
        [ "DynaMix as an entity-component-system ", "index.html#ecs", null ],
        [ "Comparison with COM ", "index.html#com", null ],
        [ "Comparison with traits ", "index.html#traits", null ],
        [ "Comparison with Ruby mixins ", "index.html#ruby", null ]
      ] ]
    ] ],
    [ "Basic Usage", "basic.html", null ],
    [ "Using DynaMix", "using.html", [
      [ "Platforms ", "using.html#platforms", null ],
      [ "Dependencies ", "using.html#deps", null ],
      [ "Getting the library ", "using.html#getting", null ],
      [ "Building the library ", "using.html#building", [
        [ "Configuration ", "using.html#config", null ]
      ] ],
      [ "Using the library ", "using.html#using", null ],
      [ "Contacts ", "using.html#contact", null ]
    ] ],
    [ "Glossary", "glossary.html", [
      [ "Objects ", "glossary.html#object", null ],
      [ "Mixins ", "glossary.html#mixin", null ],
      [ "Messages ", "glossary.html#message", [
        [ "Multicast messages ", "glossary.html#multicast", null ]
      ] ],
      [ "Message priority ", "glossary.html#priority", null ],
      [ "Message bids ", "glossary.html#bids", null ],
      [ "Multicast result combinators ", "glossary.html#combinators", null ],
      [ "Default message implementation ", "glossary.html#defaultimpl", null ],
      [ "Mutations ", "glossary.html#mutation", [
        [ "Mutation rules ", "glossary.html#mrules", null ]
      ] ]
    ] ],
    [ "Tutorials", "tutorials.html", [
      [ "Messages ", "tutorials.html#messages", null ],
      [ "Message bids ", "tutorials.html#message_bids", null ],
      [ "Object mutation ", "tutorials.html#obj_mutation", null ],
      [ "Mutation rules ", "tutorials.html#mut_rules", null ],
      [ "Multicast result combinators ", "tutorials.html#mult_combinators", null ],
      [ "Tips and tricks ", "tutorials.html#tips_tricks", [
        [ "Mixins with a common parent ", "tutorials.html#common_mixin_parent", null ]
      ] ],
      [ "Common problems and solutions ", "tutorials.html#common_problems", [
        [ "Compiler errors ", "tutorials.html#compiler_errors", null ],
        [ "Linker errors ", "tutorials.html#linker_errors", null ],
        [ "Runtime errors ", "tutorials.html#runtime_errors", null ]
      ] ]
    ] ],
    [ "Examples", "examples.html", [
      [ "Hello World ", "examples.html#example_hello", null ],
      [ "Basic ", "examples.html#example_basic", [
        [ "Includes ", "examples.html#basic_inc", null ],
        [ "Code ", "examples.html#basic_code", null ]
      ] ],
      [ "Multi DLL ", "examples.html#multi_dll", null ]
    ] ],
    [ "Debugging with DynaMix", "debugging.html", [
      [ "Watching dynamix::object instances ", "debugging.html#watching_obj", [
        [ "Inspecting what mixins are in an object ", "debugging.html#inspect_mixins", null ],
        [ "Inspecting a concrete mixin ", "debugging.html#inspect_mixin_data", null ],
        [ "Inspecting the messages in an object ", "debugging.html#inspect_msgs", null ]
      ] ],
      [ "Stepping into messages ", "debugging.html#step_into", null ],
      [ "Tracing/Logging information with code ", "debugging.html#tracing_logging", null ]
    ] ],
    [ "Advanced Topics", "advanced.html", [
      [ "Performance ", "advanced.html#performance", [
        [ "Message performance ", "advanced.html#msg_perf", null ],
        [ "Mutation performance ", "advanced.html#mutation_perf", null ]
      ] ],
      [ "Thread safety ", "advanced.html#thread_safety", null ],
      [ "Using custom allocators ", "advanced.html#using_allocators", null ],
      [ "Having messages with more arguments ", "advanced.html#more_msg_arity", null ],
      [ "Shared libraries and program plugins ", "advanced.html#dynlibs", null ],
      [ "Serializing objects ", "advanced.html#serializing", null ],
      [ "Implementation notes ", "advanced.html#impl_notes", [
        [ "Mixin and feature registration ", "advanced.html#registration", null ],
        [ "Mixin and message id-s ", "advanced.html#mix_msg_ids", null ],
        [ "Mutation and type information ", "advanced.html#mut_type_info", null ],
        [ "Calling messages ", "advanced.html#calling_msgs", null ]
      ] ],
      [ "Macros rationale ", "advanced.html#why_macros", [
        [ "Mixin definition and declaration macros ", "advanced.html#why_mixin_macros", null ],
        [ "Message definition and declaration macros ", "advanced.html#why_msg_macros", null ],
        [ "External custom preprocessor ", "advanced.html#external_moc", null ]
      ] ]
    ] ],
    [ "Revision History", "revisions.html", null ],
    [ "Roadmap", "roadmap.html", null ],
    [ "Appendix", "appendix.html", [
      [ "Appendix A. Entity-Component-System ", "appendix.html#aecs", [
        [ "What is an entity-component-system ", "appendix.html#what_is_ecs", null ],
        [ "Why are entity-component-systems good? ", "appendix.html#why_is_ecs_good", null ],
        [ "Non-polymorphic entity-component-systems ", "appendix.html#nonpoly_ecs", null ],
        [ "Who uses entity-component-systems? ", "appendix.html#who_uses_ecs", null ],
        [ "Further reading ", "appendix.html#more_ecs", null ]
      ] ],
      [ "Appendix B. Visual Studio watch visualizers ", "appendix.html#app_vs_watch", [
        [ "Watching a mixin ", "appendix.html#watch_mixin", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", null, [
      [ "File List", "files.html", "files" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
".html",
"classdynamix_1_1mutation__rule.html#a22b9b5bf66f70dc93290fd618e44afab",
"mixin__type__info_8hpp.html#aed0eb0c538fea46010b63b5a0b6f37e8"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';