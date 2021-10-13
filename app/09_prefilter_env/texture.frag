#version 330 core

out vec4 f_Color;

in vec3 v_Normal;

uniform int u_roughness;
uniform samplerCube u_cube;

void main() {       
  vec3 dir = normalize(v_Normal);
  vec3 color = textureLod(u_cube, dir, u_roughness).rgb;
  color = color / (color + vec3(1.0));
  f_Color = vec4(color, 1.0);
}