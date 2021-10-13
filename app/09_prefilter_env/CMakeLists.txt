cmake_minimum_required(VERSION 3.8)

add_executable(09_PrefilterEnv main.cpp)
CopyFilesToOutDirOnPostBuild(09_PrefilterEnv 
  texture.vert
  texture.frag
  skybox.vert
  skybox.frag)
target_link_libraries(09_PrefilterEnv HikariCommon)