cmake_minimum_required(VERSION 3.8)

add_executable(11_PbrEnv main.cpp)
CopyFilesToOutDirOnPostBuild(11_PbrEnv
  skybox.vert
  skybox.frag
  cook_torrance.vert
  cook_torrance.frag)
target_link_libraries(11_PbrEnv HikariCommon)