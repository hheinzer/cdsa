# Arena Friendly Data Structures

This repository provides general purpose data structures that are useful when programming in C. Each
data structure is implemented as a single header that only depends on the C standard library (and
possibly other headers from this repository). Example usage is provided in the source files with the
same names.

Any C23 compliant C compiler should be able to handle the data structures without any special flags.

The employed “error handling strategy” is `assert`, which might not be compatible with your project
if you want to be able to recover from errors.

Data structures manage their memory using [arena
allocators](https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator), which streamline
memory management by grouping allocations together and freeing them all at once. I first learned
about this type of allocator [here](https://nullprogram.com/blog/2023/09/27/) which made me want to
try it out for myself.

Some creation parameters are optional and accept `0` or `nullptr`. For example, if you want to
create a list, but never need to compare list items, you don't need to pass a comparison function.

Even though I'm big sucker for performance, the primary focus of this project is flexibility.
Therefore, the stored data is a `void *` pointer and the user needs to provide the size of the data
and an appropriate copy function. If you are not storing anything complicated, or you are fine with
shallow copies, the default copy function `arena_memcpy()` is your friend.

## Features

- Memory
    - `arena.h`: arena allocator
- Linear
    - `list.h`: doubly linked list
- Hashing
    - `dict.h`: associative array
    - `set.h`: set
- Trees
    - `heap.h`: binary heap

## Contributing

Contributions are welcome! If you find a bug or have a feature request, please open an issue. To
contribute code:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Commit your changes and push to your fork.
4. Open a pull request to the main branch.

## Usage

- [Me](https://github.com/hheinzer/advent-of-code-c/tree/main/2024) for [Advent of
  Code](https://adventofcode.com/) in 2024

## License

The code in this repository is licensed under the GPL-3.0 license.
