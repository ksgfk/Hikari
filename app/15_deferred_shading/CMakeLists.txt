cmake_minimum_required(VERSION 3.8)

add_executable(15_def_shade main.cpp)
CopyFilesToOutDirOnPostBuild(15_def_shade
  gbuffer.vert
  gbuffer.frag
  light.vert
  light.frag)
target_link_libraries(15_def_shade HikariCommon)