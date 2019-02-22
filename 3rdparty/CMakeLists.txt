# set Box2D options
set(BOX2D_BUILD_EXAMPLES OFF CACHE INTERNAL "Don't build examples")
set(BOX2D_INSTALL OFF CACHE INTERNAL "Don't install")

add_subdirectory(Box2D/Box2D)
set(BOX2D_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/Box2D/Box2D PARENT_SCOPE)

add_subdirectory(cnpy)
set(CNPY_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} PARENT_SCOPE)

set(PYBIND11_PYTHON_VERSION 3.6)
add_subdirectory(pybind11)