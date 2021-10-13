#version 330 core

#include <HikariTransform.glsl>

in vec3 a_Pos;
in vec2 a_TexCoord0;

out vec2 v_TexCoord;

void main() {
  v_TexCoord = a_TexCoord0;
  gl_Position = HikariObjectToClipPos(a_Pos);
}