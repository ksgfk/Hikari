#version 330 core

#include <HikariTransform.glsl>

in vec3 a_Pos;

out vec3 v_Pos;

void main() {
  v_Pos = HikariObjectToWorldPos(a_Pos);
  gl_Position = HikariObjectToClipPos(a_Pos);
}