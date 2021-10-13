#version 330 core

out vec4 f_Color;

in vec3 v_TexCoord;

uniform samplerCube u_skybox;

void main() {    
  vec3 Lo = texture(u_skybox, v_TexCoord).rgb;
  vec3 color = Lo / (Lo + vec3(1.0));  //tonemapping
  f_Color = vec4(color, 1.0);
}