cmake_minimum_required(VERSION 3.8)

add_executable(05_MultiLight main.cpp)
CopyFilesToOutDirOnPostBuild(05_MultiLight 
  blinn.vert 
  blinn.frag)
target_link_libraries(05_MultiLight HikariCommon)