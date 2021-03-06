project(bouncing_balls)
cmake_minimum_required(VERSION 3.1.0)

add_subdirectory(3rdparty)

include_directories(
	${CMAKE_SOURCE_DIR}/include 
	${BOX2D_INCLUDE_DIR}
	${CNPY_INCLUDE_DIR}
)

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
target_compile_features(bouncing_balls PRIVATE cxx_std_17)
