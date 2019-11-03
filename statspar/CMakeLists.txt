cmake_minimum_required(VERSION 3.14)
project(statspar_master C)

set(CMAKE_C_STANDARD 99)

SET(CMAKE_C_COMPILER mpicc)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-fopenmp -Wall")

add_executable(statspar_master statspar_master.c)
target_link_libraries(statspar_master m)

add_executable(statspar_slave statspar_slave.c)
target_link_libraries(statspar_slave m)

add_custom_command(TARGET statspar_master
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:statspar_master> ..)

add_custom_command(TARGET statspar_slave
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:statspar_slave> ..)