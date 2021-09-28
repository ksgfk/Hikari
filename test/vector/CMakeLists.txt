cmake_minimum_required(VERSION 3.8)

add_executable(TestVector "test_vector.cpp")
target_link_libraries(TestVector HikariCommon)
add_test(NAME TestVectorRun COMMAND TestVector)

add_executable(TestMatrix "test_matrix.cpp")
target_link_libraries(TestMatrix HikariCommon)
add_test(NAME TestMatrixRun COMMAND TestMatrix)

add_executable(TestQua "test_qua.cpp")
target_link_libraries(TestQua HikariCommon)
add_test(NAME TestQuaRun COMMAND TestQua)