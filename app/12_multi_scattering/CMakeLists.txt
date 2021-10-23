cmake_minimum_required(VERSION 3.8)

add_executable(12_MultiScattering main.cpp)
CopyFilesToOutDirOnPostBuild(12_MultiScattering
  skybox.vert
  skybox.frag
  cook_torrance.vert
  cook_torrance.frag)
target_link_libraries(12_MultiScattering HikariCommon)