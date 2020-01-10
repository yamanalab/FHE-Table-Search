# Function Evaluation with FHE using Table Lookup

This is a demo of function evaluation of fully homomorphic encryption using table lookup. Including two files, one is for one-input function, the other is for two-input function.

# This is a demo

# Contents
- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [Building](#building)
- [Run](#run)

# Introduction
Tested on Ubuntu and CentOS

# Prerequisites
Building in MacOS or Linux using CMake(>=3.10), GUN g++(>=6.0) is needed.
- [Microsoft/SEAL version 3.2.0](https://github.com/microsoft/SEAL/tree/3.2.0)
- GUN g++
- CMake

# Building
For two-input function, please install Microsoft/SEAL in file `two-input`.
You can edit:
`make_table.cpp` to change the table size,
`demo-two-ds1.cpp` to change the parameters of FHE,
`demo-two-in.cpp` to change the input number.
When you change the table size, do not forget to change the "NUM" and "NUM2" in `server.hpp`.

```
cmake .
make
```
If you change the table size,
```
g++ make_table.cpp -o make_table
./make_table
```

# Run
```
bin/sealds1
bin/sealin
bin/sealcs1 query
bin/sealds2 query
bin/sealcs2 query result
```

