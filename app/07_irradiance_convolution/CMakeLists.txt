cmake_minimum_required(VERSION 3.8)

add_executable(07_IrradianceConvolution main.cpp)
CopyFilesToOutDirOnPostBuild(07_IrradianceConvolution 
  texture.vert
  texture.frag
  skybox.vert
  skybox.frag)
target_link_libraries(07_IrradianceConvolution HikariCommon)