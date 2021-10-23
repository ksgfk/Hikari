cmake_minimum_required(VERSION 3.8)

add_executable(13_NormalMap main.cpp)
CopyFilesToOutDirOnPostBuild(13_NormalMap
  pbr.vert
  pbr.frag)
target_link_libraries(13_NormalMap HikariCommon)