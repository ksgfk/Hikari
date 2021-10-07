# CMakeList.txt: Hikari 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required(VERSION 3.8)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(HIKARI_BUILD_SHARED_DEFAULT OFF)
option(HIKARI_BUILD_SHARED "Build Hikari as a shared library?" ${HIKARI_BUILD_SHARED_DEFAULT})

set(HIKARI_UNIT_TEST_DEFAULT ON)
option(HIKARI_UNIT_TEST "Build Hikari unit test?" ${HIKARI_UNIT_TEST_DEFAULT})

# 将传入的所有文件复制到目标，FilesName是可变参数
function(CopyFilesToOutDirOnPostBuild TargetCmd FilesName)
  set(THIS_IDX 1)
  while(THIS_IDX LESS ${ARGC})
	list(APPEND THIS_FILES_LIST "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV${THIS_IDX}}")
    math(EXPR THIS_IDX "${THIS_IDX} + 1")
  endwhile()
  add_custom_command(TARGET ${TargetCmd} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different 
	${THIS_FILES_LIST}
	$<TARGET_FILE_DIR:${TargetCmd}>)
endfunction()

# 将文件复制到目标
function(CopyFileToOutDirOnPostBuild TargetCmd FilesName)
add_custom_command(TARGET ${TargetCmd} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    ${CMAKE_CURRENT_SOURCE_DIR}/${FileName}
    $<TARGET_FILE_DIR:${TargetCmd}>)
endfunction()

project("Hikari")

if (HIKARI_BUILD_SHARED)#HIKARI_LIBRARY_TYPE字段，表示公共库类型
  set(HIKARI_LIBRARY_TYPE "SHARED")
else()
  set(HIKARI_LIBRARY_TYPE "STATIC")
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS")#关闭CRT安全警告
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")#全部使用utf-8
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Complier clang-cl")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    message(STATUS "Complier MSVC")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  message(STATUS "Complier Clang")
endif()

set(HikariInclude ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 依赖库
add_subdirectory(ext)
# 公共库
add_subdirectory(src)
# 应用
add_subdirectory(app)
# 单元测试
if(HIKARI_UNIT_TEST)
include(CTest)
enable_testing()
add_subdirectory(test)
endif()