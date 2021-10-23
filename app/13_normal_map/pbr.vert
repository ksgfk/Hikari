#version 330 core

#include <HikariTransform.glsl>

in vec3 a_Pos;
in vec4 a_Tangent;
in vec3 a_Normal;
in vec2 a_TexCoord0;

out vec3 v_Pos;
out vec2 v_TexCoord;
out mat3 v_TBN;

void main() {
  v_Pos = HikariObjectToWorldPos(a_Pos);
  v_TexCoord = a_TexCoord0;
  v_TBN = TBN(a_Normal, a_Tangent);
  gl_Position = HikariObjectToClipPos(a_Pos);
}