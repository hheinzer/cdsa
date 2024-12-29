# Common Data Structures and Algorithms for C

This repository provides general purpose data structures and algorithms that are often needed when
programming in C. Each data structure is implemented as a single header that only depends on the C
standard library (and possibly other headers from this repository). Most data structures are
semantically identical to their Python counterparts with the same name. Example usage is provided in
the source files with the same names.

Any C23 compliant C compiler should be able to handle the data structures without any special flags.
Most of the code should also compile under C99, but for anything lower, you might need to get your
hands dirty.

The employed “error handling strategy” is `assert` and `abort`. This might not be compatible with
your project if you want to be able to recover from errors.

Data structures manage their memory using
[arena allocators](https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator), which
streamline memory management by grouping allocations together and freeing them all at once. I first
learned about this type of allocator [here](https://nullprogram.com/blog/2023/09/27/) and was
immediately hooked by their elegance and simplicity. You may still use traditional heap
allocation for the actual data if dynamic resizing or longer lifetimes are required.

Some creation parameters are optional and accept a literal `0` (or null pointer). For example, if
you want to create a list, but never need to compare list items, you don't need to pass a comparison
function.

Even though I'm big sucker for performance, the primary focus of this project is flexibility.
Therefore, the stored data is always a `void` pointer and the user needs to provide the size of the
data and appropriate copy and destroy functions. If you are not storing anything complicated,
`memcpy` and `free` are your friends. If you use an arena allocator for your data, you probably
won't need a destroy function.

## Features

- Memory
    - [x] `dump.h`: inspect memory buffers
    - [x] `arena.h`: arena allocator
    - [ ] `fatptr.h`: fat pointers (or non-null terminated strings)
- Linear
    - [x] `list.h`: doubly linked list
- Hashing
    - [ ] `dict.h`: associative array using open chaining
    - [ ] `hmap.h`: associative array using open addressing
    - [ ] `set.h`: set using hashing and open addressing
- Trees
    - [ ] `trie.h`: prefix tree (dynamic hash map)
    - [ ] `heap.h`: priority queue
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

- [Me](https://github.com/hheinzer/advent-of-code-c) for [Advent of Code](https://adventofcode.com/)
  in 2024 (and beyond)


## License

The code in this repository is licensed under the GPL-3.0 [license](LICENSE).
