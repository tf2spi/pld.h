# pld.h
Functions and logging generally useful for preloads

## Why Make This Header?

I do a lot of preloads in general to debug and mess with applications.
That makes it useful to have a quick and dirty header file I can just
include in a single file in order to easily fetch loaded modules
and get their respective functions.

## Usage

You can just include the header file in your preload main source file.
All the functions in the header are static so keep that in mind when
your preload spans multiple files.

The following macros are defined to load libraries and symbols

```
PLD(name)
* Use the library with this name when loading functions

PLDH
* PLD(name) defines a function with this name
* You can use this macro to redefine the preloaded library if preloading multiple libraries

PFNALTFIND(name, alt)
* Use the type signature of the variable called "name", but use the string "alt" to look it up
* This is useful for ordinals in Windows or typecasting in general

PFNFIND(name)
* Use both the string name and type signature of the variable called "name"
* Equivalent to PFNALTFIND(name, #name)

PFNALT(h, name, alt)
* Like PFNALTFIND, but cache the result in a new static variable named via "h"

PFN(h, name)
* Like PFNFIND but cache the result in a new static variable named via "h"
```

Usages are in the ``examples`` directory

## Why Use This Header?

* Type signatures don't need to be typed out when hooking functions.
* Saves boilerplate needed to load the libraries/functions while checking the return values.
* Works with the Following Compilers
  - MSVC
  - GCC
  - Clang
  - TCC
* Works with C and C++
* Comes with logging using only ``write`` on POSIX or ``WriteFile`` on Windows by default
  - Logging can be overridden to be another function or not even present, see ``write.c`` in ``examples``
  - Not essential but it helps diagnose silly things like spelling errors very quickly.

