cmake_minimum_required(VERSION 3.15)
project(ex1 C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-L. -lmtm ")

add_executable(ex1
        amount_set.h
        amount_set.c
        matamazom.h
        matamazom.c
        set.h
        libmtm.a
        matamazom_print.h
        matamazom_print.c
        #amount_set_main.c
        #amount_set_tests.h
        #amount_set_tests.c)
        #test_utilities.h)
        #main-idan.c
        main.c)
        #matamazom_main.c
        #matamazom_tests.h
        #matamazom_tests.c)
        #unofficialtestAmountSet.c)

target_link_libraries(ex1 ${CMAKE_SOURCE_DIR}/libmtm.a)