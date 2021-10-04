cmake_minimum_required(VERSION 3.8)

if (HIKARI_BUILD_SHARED)
  message(STATUS "Hikari Common: shared lib.")
else()
  message(STATUS "Hikari Common: static lib.")
endif()

add_library(HikariCommon ${HIKARI_LIBRARY_TYPE} 
  "common.cpp"
  "camera.cpp"
  "input.cpp"
  "window.cpp"
  "asset.cpp"
  "render_context.cpp"
  "opengl.cpp"
  "application.cpp")

if (HIKARI_BUILD_SHARED)#定义HIKARI_SHARED宏
target_compile_definitions(HikariCommon PUBLIC -DHIKARI_SHARED)
endif()
target_link_libraries(HikariCommon glad)
target_link_libraries(HikariCommon glfw)
target_link_libraries(HikariCommon stb_lib)
target_link_libraries(HikariCommon tinyobjloader)
target_link_libraries(HikariCommon glslang OSDependent OGLCompiler SPIRV SPVRemapper)
target_link_libraries(HikariCommon spirv-cross-core spirv-cross-glsl spirv-cross-util)
# glfw import include folder automatically
target_include_directories(HikariCommon PUBLIC ${HikariInclude}) #hikari include dir
glslang_add_build_info_dependency(HikariCommon) #glslang include dir