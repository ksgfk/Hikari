cmake_minimum_required(VERSION 3.8)

add_executable(14_IntImgui main.cpp)
CopyFilesToOutDirOnPostBuild(14_IntImgui
  pbr.vert
  pbr.frag)
target_link_libraries(14_IntImgui HikariCommon)