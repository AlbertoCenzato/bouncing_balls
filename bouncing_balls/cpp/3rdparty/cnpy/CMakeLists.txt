cmake_minimum_required(VERSION 3.0)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

add_library(CNPY_LIB STATIC
	cnpy.cpp
)