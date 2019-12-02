cmake_minimum_required(VERSION 3.15)
project(ex1 C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-L. -lmtm ")


include_directories(.)


add_executable(ex1
        amount_set.h
        amount_set.c
        matamazom.c
        matamazom.h
        matamazom_print.h
        matamazom_print.c
        set.h
        main.c
        )

target_link_libraries(ex1 ${CMAKE_SOURCE_DIR}/libmtm.a)