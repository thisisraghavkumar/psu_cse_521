# CSE 521: Compiler Construction
## Programming Assignment 1 : Value Numbering

Submitted by-
1. Durva Dev (dbd5616)
2. Raghav Kumar (rfk5511)

### To run

**Compile for C++20**

`g++ --std=c++20 -stdlib=libc++ src/value_numbering.cpp -o bin/vn`

**Run with filename**

`bin/vn examples/test3`

*Note:* This will produce an output file with name *test3_vno* in same directory as file *test3*.

**Statements must be of the format** `x := y + z;`

**Included binary bin/vn has been compiled for M1 Mac.**

### About code

All code is in file *src/value_numbering.cpp* with the main function as entry point. Comments have been added in the source code to explain the logic.