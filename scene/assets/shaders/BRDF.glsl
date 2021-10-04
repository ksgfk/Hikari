#ifndef HIKARI_BRDF_INCLUDED
#define HIKARI_BRDF_INCLUDED

#include <Macros.glsl>

struct PbrMaterial {
  vec3 AlbedoMap;
  float Metallic;
  float Roughness;
};

float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float nDotH = dot(N, H);
  nDotH = clamp(nDotH, 0.0, 1.0);
  float b = nDotH * nDotH * (a2 - 1.0) + 1.0;
  return a2 / (PI * b * b);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  NdotV = clamp(NdotV, 0.0, 1.0);
  float x = (roughness + 1.0);
  float k = x * x / 8.0;
  return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float nDotL = clamp(dot(N, L), 0.0, 1.0);
  float nDotV = clamp(dot(N, V), 0.0, 1.0);
  float g1 = GeometrySchlickGGX(nDotL, roughness);
  float g2 = GeometrySchlickGGX(nDotV, roughness);
  return g1 * g2;
}

vec3 FresnelSchlick(vec3 F0, vec3 V, vec3 H) {
  float theta = dot(V, H);
  theta = clamp(theta, 0.0, 1.0);
  return F0 +(1.0 - F0) * pow(1.0 - theta, 5.0);
}

#endif