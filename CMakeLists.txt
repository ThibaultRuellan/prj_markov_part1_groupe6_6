cmake_minimum_required(VERSION 4.0)
project(untitled C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled main.c
        utils.c
        tarjan.c
        matrix.c
        graph.c
        graph.h
        tarjan.h
        matrix.h
        utils.h
        hasse.h
        hasse.c
)
