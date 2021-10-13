#version 330 core

out vec4 f_Color;

in vec2 v_TexCoord;

uniform sampler2D u_texture;

void main() {       
  vec3 color = texture(u_texture, v_TexCoord).rgb;
  f_Color = vec4(color, 1.0);
}