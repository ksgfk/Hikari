#version 330 core

in vec3 a_Pos;

out vec3 v_TexCoord;

uniform mat4 projection;
uniform mat4 view;

void main() {
  v_TexCoord = a_Pos;
  vec4 pos = projection * view * vec4(a_Pos, 1.0);
  gl_Position = pos.xyww;
}