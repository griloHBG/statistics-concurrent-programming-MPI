cmake_minimum_required(VERSION 3.14)
project(studentspar C)

set(CMAKE_C_STANDARD 99)

OPTION (USE_OpenMP "Use OpenMP" ON)
IF(USE_OpenMP)
    FIND_PACKAGE(OpenMP REQUIRED)
    IF(OPENMP_FOUND)
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    ENDIF()
ENDIF()

add_executable(studentspar studentspar.c)
#target_link_libraries(studentspar m)
