# Common Data Structures for C

This repository provides general purpose data structures that are often needed when programming in
C. Each data structure is implemented as a single header that only depends on the C standard
library. Most data structures are semantically identical to their Python counterparts with the same
name. Example usage is provided in the source files with the same names. Any C99 compliant C
compiler should be able to handle the data structures without any special flags.

The employed “error handling strategy” is heavy usage of `assert`. This might not be compatible with
your project if you want to be able to recover from errors.
