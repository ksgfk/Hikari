#version 330 core

out vec4 f_Color;

in vec3 v_Pos;

uniform sampler2D u_equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v) {
  vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
  uv *= invAtan;
  uv += 0.5;
  return uv;
}

void main() {       
  vec2 uv = SampleSphericalMap(normalize(v_Pos));
  vec3 color = texture(u_equirectangularMap, uv).rgb;
  f_Color = vec4(color, 1.0);
}