cmake_minimum_required(VERSION 3.8)
#glad
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glad)
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

#glfw
set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glfw)
add_subdirectory(${GLFW_DIR})
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#stb
set(STB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stb)
set(HIKARI_COMPILE_STB_DEFAULT ON)
option(HIKARI_COMPILE_STB "Build stb?" ${HIKARI_COMPILE_STB_DEFAULT})
add_library(stb_lib STATIC stb_source.cpp)
if (HIKARI_COMPILE_STB)
target_compile_definitions(stb_lib PRIVATE -DHIKARI_COMPILE_STB)
endif()
target_include_directories(stb_lib PUBLIC ${STB_DIR})

#tinyobjloader
set(TINY_OBJ_LOADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader)
add_library(tinyobjloader STATIC ${TINY_OBJ_LOADER_DIR}/tiny_obj_loader.cc)
target_include_directories(tinyobjloader PUBLIC ${TINY_OBJ_LOADER_DIR})