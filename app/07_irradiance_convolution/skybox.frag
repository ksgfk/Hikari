#version 330 core

out vec4 f_Color;

in vec3 v_TexCoord;

uniform samplerCube u_skybox;

void main() {    
  f_Color = texture(u_skybox, v_TexCoord);
}