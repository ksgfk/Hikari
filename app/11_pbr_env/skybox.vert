#version 330 core

#include <HikariTransform.glsl>

in vec3 a_Pos;

out vec3 v_TexCoord;

void main() {
  v_TexCoord = a_Pos;
  vec4 pos = u_MatrixP * mat4(mat3(u_MatrixV)) * vec4(a_Pos, 1.0);
  gl_Position = pos.xyww;
}