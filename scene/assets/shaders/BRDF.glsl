#ifndef HIKARI_BRDF_INCLUDED
#define HIKARI_BRDF_INCLUDED

#include <Macros.glsl>

struct MetallicWorkflowMaterial {
  vec3 Albedo;
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
  return F0 + (1.0 - F0) * pow(1.0 - theta, 5.0);
}

struct BlinnPhongMaterial {
  vec3 kd;
  vec3 ks;
  float shiness;
};

/**
 * norm:法线方向
 * LiDir:光源方向
 * LiRad:光源radiance
 * eyePos:摄像机位置
 * shadePos:着色点
 */
vec3 BlinnPhong(BlinnPhongMaterial blinn, vec3 norm, vec3 LiDir, vec3 LiRad, vec3 eyePos, vec3 shadePos) {
  vec3 kd = blinn.kd;
  vec3 ks = blinn.ks;
  float shin = blinn.shiness;

  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);

  float NdotL = dot(LiDir, norm);
  diffuse = max(NdotL, 0.0) * LiRad * kd;

  vec3 viewDir = normalize(eyePos - shadePos);
  vec3 halfDir = normalize(viewDir + LiDir);
  float spec = max(pow(dot(halfDir, norm), shin), 0);
  specular = ks * LiRad * spec;

  return diffuse + specular;
}

#endif