#version 330 core

#include <HikariTransform.glsl>

in vec3 a_Pos;
in vec3 a_Normal;

out vec3 v_Normal;

void main() {
  v_Normal = HikariWorldNormal(a_Normal);
  gl_Position = HikariObjectToClipPos(a_Pos);
}