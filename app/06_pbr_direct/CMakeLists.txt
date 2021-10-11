cmake_minimum_required(VERSION 3.8)

add_executable(06_PbrDirect main.cpp)
CopyFilesToOutDirOnPostBuild(06_PbrDirect 
  blinn.vert
  blinn.frag
  cook_torrance.vert
  cook_torrance.frag)
target_link_libraries(06_PbrDirect HikariCommon)