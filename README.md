# Function Evaluation with FHE using Lookup Table

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
For one-input function, please install Microsoft/SEAL in file `one-input`.
You can edit:
`make_table.cpp` to change the table size,
`demo-two-ds1.cpp` to change the parameters of FHE,
`demo-two-in.cpp` to change the input number.
When you change the table size, do not forget to change the "TABLE_SIZE_N" in `server.hpp`. 

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
g++ make_table.cpp -o make_tale
./make_table
```

# Run
This is a demo test, the input values and table is not real function. We decide the input value and a demo table. In one-input situation the input number is 2, and in two-input situation the input numbers are (1,1).

one-input
```
bin/sealds1
bin/sealin
bin/sealcs1 query
bin/sealds2 query
bin/sealcs2 query
```

two-input
```
bin/sealds1
bin/sealin
bin/sealcs1 query
bin/sealds2 query
bin/sealcs2 query result
```
