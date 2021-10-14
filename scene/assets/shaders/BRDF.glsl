#ifndef HIKARI_BRDF_INCLUDED
#define HIKARI_BRDF_INCLUDED

#include <Macros.glsl>

struct MetallicWorkflowMaterial {
  vec3 Albedo;
  float Metallic;
  float Roughness;
};

//GGX法线分布
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float nDotH = dot(N, H);
  nDotH = clamp(nDotH, 0.0, 1.0);
  float b = nDotH * nDotH * (a2 - 1.0) + 1.0;
  return a2 / (PI * b * b);
}

//G1 GGX几何遮蔽Schlick近似
float GeometrySchlickGGX(float NdotV, float roughness) {
  NdotV = clamp(NdotV, 0.0, 1.0);
  float x = (roughness + 1.0);
  float k = x * x / 8.0;
  return NdotV / (NdotV * (1.0 - k) + k);
}

//GGX几何遮蔽
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float nDotL = clamp(dot(N, L), 0.0, 1.0);
  float nDotV = clamp(dot(N, V), 0.0, 1.0);
  float g1 = GeometrySchlickGGX(nDotL, roughness);
  float g2 = GeometrySchlickGGX(nDotV, roughness);
  return g1 * g2;
}

//菲涅尔项，Schlick近似
vec3 FresnelSchlick(vec3 F0, vec3 V, vec3 H) {
  float theta = dot(V, H);
  theta = clamp(theta, 0.0, 1.0);
  float b = 1.0 - theta;
  float b5 = b * b * b * b * b; //pow(b, 5.0)
  return F0 + (1.0 - F0) * b5;
}

//菲涅尔项，Schlick近似
vec3 FresnelSchlickRoughness(vec3 F0, vec3 V, vec3 H, float roughness) {
  float theta = dot(V, H);
  theta = clamp(theta, 0.0, 1.0);
  float b = 1.0 - theta;
  float b5 = b * b * b * b * b;
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * b5;
}

// 低差异序列
vec2 Hammersley(uint i, uint N) {
  uint bits = i;
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  float rdi = float(bits) * 2.3283064365386963e-10; // / 0x100000000
  return vec2(float(i) / float(N), rdi);
}

// GGX重要性采样
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
  float a = roughness * roughness;
  //https://google.github.io/filament/Filament.html#toc9.3
  float phi = 2.0 * PI * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);

  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
  return normalize(sampleVec);
}

//---------------https://google.github.io/filament/Filament.html#listing_glslbrdf----------------
float D_GGX(float NoH, float a) {
  float a2 = a * a;
  float f = (NoH * a2 - NoH) * NoH + 1.0;
  return a2 / (PI * f * f);
}

vec3 F_Schlick(float u, vec3 f0) {
  return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
  float a2 = a * a;
  float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
  float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
  return 0.5 / (GGXV + GGXL);
}

float Fd_Lambert() {
  return INV_PI;
}

vec3 Cook_Torrance(vec3 v, vec3 l, vec3 n, vec3 f0, float rough, vec3 albedo, float metallic) {
  vec3 h = normalize(v + l);
  float NoV = abs(dot(n, v)) + 1e-5;
  float NoL = clamp(dot(n, l), 0.0, 1.0);
  float NoH = clamp(dot(n, h), 0.0, 1.0);
  float LoH = clamp(dot(l, h), 0.0, 1.0);

  float roughness = rough * rough;

  float D = D_GGX(NoH, roughness);
  vec3  F = F_Schlick(LoH, f0);
  float V = V_SmithGGXCorrelated(NoV, NoL, roughness);

  vec3 Fr = (D * V) * F;

  vec3 ks = F;
  vec3 kd = (vec3(1.0) - ks) * (1.0 - metallic);
  vec3 Fd = kd * albedo * Fd_Lambert();

  return (Fr + Fd) * NoL;
}
//-----------------------------------------------------------------------------------------------

struct BlinnPhongMaterial {
  vec3 kd;
  vec3 ks;
  float shiness;
};

/**
 * Diffuse漫反射+BlinnPhong高光
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