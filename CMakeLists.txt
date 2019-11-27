cmake_minimum_required(VERSION 3.15)
project(ex1 C)

set(CMAKE_C_STANDARD 99)

add_executable(ex1 main.c Matamazon/matamazom.c amount_Set/amount_set.c)