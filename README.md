# Common Data Structures and Algorithms for C

This repository provides general purpose data structures and algorithms that are often needed when
programming in C. Each data structure is implemented as a single header that only depends on the C
standard library (and possibly a base data structure). Most data structures are semantically
identical to their Python counterparts with the same name. Example usage is provided in the source
files with the same names.

Any C23 compliant C compiler should be able to handle the data structures without any special flags.
Most of the code should also compile under C99, but for anything lower, you might need to get your
hands dirty.

The employed “error handling strategy” is heavy usage of `assert`. This might not be compatible with
your project if you want to be able to recover from errors.

All memory is allocated through `malloc`, `calloc`, and `realloc`. If you want to use a custom
allocator, use macros to redefine these functions, or adjust the header files directly.

Some creation parameters are optional and accept a literal `0` (or null pointer). For example, if
you want to create a list, but never need to compare list items, you don't need to pass a comparison
function.

Even though I'm big sucker for performance, the primary focus of this project is flexibility.
Therefore, the stored data is always a `void` pointer and the user needs to provide the size of the
data and appropriate copy and deallocation function. If you are not storing anything complicated,
`memcpy` and `free` are your friends.

## Features

- Linear Data Structures
    - [x] `array.h`: dynamic array
    - [x] `list.h`: linked list
- Hashing
    - [x] `hash.h`: hash functions
    - [x] `dict.h`: associative array using open chaining
    - [x] `hmap.h`: associative array using open addressing
    - [x] `set.h`: set using hashing and open addressing
- Trees
    - [x] `heap.h`: priority queue
- Graphs
    - [ ] `graph.h`

## Contributing

Contributions are welcome! If you find a bug or have a feature request, please open an issue. To
contribute code:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Commit your changes and push to your fork.
4. Open a pull request to the main branch.

## Usage

- [Me](https://github.com/hheinzer/advent-of-code-c) (not yet, but soon) for [Advent of
  Code](https://adventofcode.com/)


## License

The code in this repository is licensed under the GPL-3.0 [license](LICENSE).
