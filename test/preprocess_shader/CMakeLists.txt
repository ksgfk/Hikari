cmake_minimum_required(VERSION 3.8)

add_executable(TestPreprocessShader "test_preprocess_shader.cpp")
target_link_libraries(TestPreprocessShader HikariCommon)
add_test(NAME TestPreprocessShaderRun COMMAND TestPreprocessShader)

add_executable(TestParseShader "test_parse_shader.cpp")
target_link_libraries(TestParseShader HikariCommon)
add_test(NAME TestParseShaderRun COMMAND TestParseShader)