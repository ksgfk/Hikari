cmake_minimum_required(VERSION 3.8)

add_executable(08_DiffuseIrradiance main.cpp)
CopyFilesToOutDirOnPostBuild(08_DiffuseIrradiance 
  texture.vert
  texture.frag
  skybox.vert
  skybox.frag
  cook_torrance.vert
  cook_torrance.frag)
target_link_libraries(08_DiffuseIrradiance HikariCommon)