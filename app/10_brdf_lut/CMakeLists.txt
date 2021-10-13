cmake_minimum_required(VERSION 3.8)

add_executable(10_BrdfLut main.cpp)
CopyFilesToOutDirOnPostBuild(10_BrdfLut 
  texture.vert
  texture.frag)
target_link_libraries(10_BrdfLut HikariCommon)