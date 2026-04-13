cmake_minimum_required(VERSION 3.9.0)
project(QSORT)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -finstrument-functions -g -gdwarf-4")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -finstrument-functions -g -gdwarf-4")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

add_library(Qsort qsort.c)

add_executable(qsort_test qsort.c)
target_link_libraries(qsort_test Qsort gcov)