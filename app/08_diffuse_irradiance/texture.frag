#version 330 core

out vec4 f_Color;

in vec3 v_Pos;

uniform samplerCube u_cube;

void main() {       
  vec3 dir = normalize(v_Pos);
  vec3 color = texture(u_cube, dir).rgb;
  f_Color = vec4(color, 1.0);
}