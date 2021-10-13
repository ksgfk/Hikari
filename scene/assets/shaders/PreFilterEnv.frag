#version 330 core

#include <BRDF.glsl>

#ifndef SAMPLE_COUNT
#define SAMPLE_COUNT 2048
#endif

out vec4 f_Color;

in vec3 v_Pos;

uniform float u_roughness;
uniform samplerCube u_Cube;

void main() {
  vec3 N = normalize(v_Pos);
  // 假设反射方向总是等于采样方向
  vec3 R = N;
  vec3 V = R;

  float weight = 0.0;
  vec3 color = vec3(0.0);
  for(uint i = 0u; i < uint(SAMPLE_COUNT); i++) {
    vec2 Xi = Hammersley(i, uint(SAMPLE_COUNT));
    vec3 H = ImportanceSampleGGX(Xi, N, u_roughness);
    vec3 L = normalize(2.0 * dot(V, H) * H - V);
    float NdotL = max(dot(N, L), 0.0);
    color += texture(u_Cube, L).rgb * NdotL * step(0, NdotL);
    weight += NdotL * step(0, NdotL);
  }
  color /= weight;

  f_Color = vec4(color, 1.0);
}