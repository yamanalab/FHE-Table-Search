cmake_minimum_required(VERSION 3.10)

set(project_name FHE-Table-Search)
project(${project_name})

find_package(Threads REQUIRED)
set(CMAKE_CXX_FLAGS "-O3 -std=c++17 -openmp -pthread -Wall -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -std=c++17 -openmp -pthread -Wall")
set(SEAL_USE_CXX17 OFF)

find_package(SEAL 3.2.0 EXACT REQUIRED)
find_package(OpenMP REQUIRED)
if(OpenMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(${PROJECT_SOURCE_DIR}/fts ${PROJECT_SOURCE_DIR}/stdsc)

set(COMMON_LIBS stdsc fts_share SEAL::seal)

add_subdirectory(stdsc)
add_subdirectory(fts)
add_subdirectory(demo)
