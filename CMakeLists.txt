project(bouncing_balls)
cmake_minimum_required(VERSION 3.1.0)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

set(BOX2D_BUILD_STATIC true)

add_subdirectory(3rdparty/cnpy)
add_subdirectory(3rdparty/Box2D)

set(PYBIND11_PYTHON_VERSION 3.6)
add_subdirectory(3rdparty/pybind11)

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/3rdparty)

file (GLOB_RECURSE GLOB_SRC	src/*.cpp)

if (CPP_EXECUTABLE)
	add_executable(${PROJECT_NAME} main.cpp ${GLOB_SRC})

	add_dependencies(${PROJECT_NAME} Box2D CNPY_LIB)
	link_directories(
		${CMAKE_BINARY_DIR}/3rdparty/Box2D 
		${CMAKE_BINARY_DIR}/3rdparty/cnpy
	)
	target_link_libraries(${PROJECT_NAME} Box2D CNPY_LIB)
endif()

pybind11_add_module(bouncing_balls wrapper_entrypoint.cpp ${GLOB_SRC})
target_link_libraries(bouncing_balls PRIVATE Box2D CNPY_LIB)
