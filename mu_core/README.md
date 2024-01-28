# `mu_core`: Fundamental support for mulib modules

This directory contains modules for use by any other mulib modules.  The basic
rule is that modules in `mu_core` may depend on other modules in `mu_core` and
`#include <stdxxx>` files, but none others.

| module | descripton |
|-----|-----|
|'mu_pque' | queue of pointer-sized objects (LIFO) |
|'mu_pvec' | vector of pointer-sized objects (FIFO) |
|'mu_spsc' | single producer, single consumer thread-safe FIFO |
|'mu_str' | safe, 'zero copy' operations on strings and slices of strings |
|'mu_istr' | safe, in-place operations on string with an index field |
|'mu_macros' | (.h only) MU_MIN, MU_MAX, MU_BYTECOUNT, etc. |
|'mu_log' | lean, versatile logging facility |

