# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

cmake_minimum_required(VERSION 3.10)

project(SEALExamples VERSION 3.2.0 LANGUAGES CXX)
project(MyOpenMPProject)

# Executable will be in bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(SEAL_USE_CXX17 OFF)

add_executable(sealds1 demo-ds1.cpp)
add_executable(sealin demo-in.cpp)
add_executable(sealcs1 demo-cs1.cpp)
add_executable(sealds2 demo-ds2.cpp)
add_executable(sealcs2 demo-cs2.cpp)
add_executable(sealout demo-out.cpp)


# Import Microsoft SEAL
find_package(SEAL 3.2.0 EXACT REQUIRED)
find_package(OpenMP REQUIRED)
if(OpenMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Link Microsoft SEAL
target_link_libraries(sealds1 SEAL::seal)
target_link_libraries(sealin SEAL::seal)
target_link_libraries(sealcs1 SEAL::seal)
target_link_libraries(sealds2 SEAL::seal)
target_link_libraries(sealcs2 SEAL::seal)
target_link_libraries(sealout SEAL::seal)
