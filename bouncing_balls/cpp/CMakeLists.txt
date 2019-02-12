project(bouncing_balls)
cmake_minimum_required(VERSION 3.0)

add_definitions(-std=c++17)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

set(BOX2D_BUILD_STATIC true)

add_subdirectory(3rdparty/cnpy)
add_subdirectory(3rdparty/Box2D)

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/3rdparty)

file (GLOB_RECURSE GLOB_SRC	src/*.cpp)

add_executable(${PROJECT_NAME} main.cpp ${GLOB_SRC})

add_dependencies(${PROJECT_NAME} Box2D CNPY_LIB)
link_directories(
	${CMAKE_BINARY_DIR}/3rdparty/Box2D 
	${CMAKE_BINARY_DIR}/3rdparty/cnpy
)
target_link_libraries(${PROJECT_NAME} Box2D CNPY_LIB)