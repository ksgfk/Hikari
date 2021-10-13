#version 330 core

in vec3 a_Pos;
in vec2 a_TexCoord0;

out vec2 v_TexCoord;

void main() {
  v_TexCoord = a_TexCoord0;
  gl_Position = vec4(a_Pos, 1.0);
}