#version 330 core

in vec3 a_Pos;

out vec3 v_Pos;

uniform mat4 projection;
uniform mat4 view;

void main() {
  v_Pos = a_Pos;
  gl_Position = projection * view * vec4(a_Pos, 1.0);
}