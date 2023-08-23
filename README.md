# Phynal, a C++ to Python Interop Library

## Why another Python/C++ Interop Library?

Reason number one: Const correctness. Existing libraries seem to just give up when confronted with a const object, and just `const_cast` the const away. This opens a door to all sorts of problems, and thus I felt a new library was in order. The name "phynal" is a play on the word "final" (and pronounced the same way).

Other minor reasons:
1. Sub-interpreter support - This does not seem to be a priority for other bindings libraries.
2. I wanted to experiment with a monadic, non-exception based approach. Converting between C++ and Python exceptions is a reasonable approach, but I wanted to see what such an API would look like.
3. Modernizing the API - Most existing libraries have a very similar API. I wanted to experiment to see if there were any changes that could unlock better performance, mental models, or simplify existing code.
4. I wanted to try it - One of the best motivators.

## Non- or Low-priority goals

Things that, in my opinion, add a lot of complexity with little to no benefit - and potentially complicate other code:

1. Function overloading - This can maybe be worked in after, but having this as a default complicates normal function calls.
2. Stateful functions - Phynal creates CPython wrapper functions from existing functions at compile time - having stateful functions makes this incredibly difficult.
