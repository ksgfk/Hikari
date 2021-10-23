#version 330 core

//https://google.github.io/filament/Filament.html#materialsystem/improvingthebrdfs/energylossinspecularreflectance

#include <BRDF.glsl>

#ifndef SAMPLE_COUNT_LUT
#define SAMPLE_COUNT_LUT 2048
#endif

#ifndef SAMPLE_COUNT_DIFF
#define SAMPLE_COUNT_DIFF 128
#endif

out vec3 FragColor;

in vec2 v_TexCoord;

float F_Schlik(float F0, float F90, float u) {
  return F0 + (F90 - F0) * pow(1.f - u, 5.0);
}

float IBL_PBR_Diffuse(float LoH, float NoL, float NoV, float Roughness) {
  float F90 = mix(0, 0.5, Roughness) + (2 * LoH * LoH * Roughness);
  return F_Schlik(1.0, F90, NoL) * F_Schlik(1.0, F90, NoV) * mix(1, 1 / 0.662, Roughness);
}

float IBL_PBR_Specular_G(float NoL, float NoV, float a) {
  float a2 = a * a;
  a2 = a2 * a2;
  float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
  float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
  return (2 * NoL) / (GGXL + GGXV);
}

vec3 CosineSampleHemisphere(float u1, float u2) {
  float r = sqrt(u1);
  float theta = 2 * PI * u2;
 
  float x = r * cos(theta);
  float y = r * sin(theta);
 
  return vec3(x, y, sqrt(max(0.0f, 1 - u1)));
}

float IBL_Default_DiffuseIntegrated(float Roughness, float NoV)
{
  vec3 V;
  V.x = sqrt(1 - NoV * NoV);
  V.y = 0;
  V.z = NoV;
    
  float r = 0;
  for (uint i = 0u; i < uint(SAMPLE_COUNT_DIFF); i++) {
    vec2 E = Hammersley(i, uint(SAMPLE_COUNT_DIFF));
    vec3 H = CosineSampleHemisphere(E.x, E.y);
    vec3 L = 2 * dot(V, H) * H - V;

    float NoL = clamp(L.z, 0.0, 1.0);
    float LoH = clamp(dot(L, H.xyz), 0.0, 1.0);

    if(LoH > 0) {
      float Diffuse = IBL_PBR_Diffuse(LoH, NoL, NoV, Roughness);
      r += Diffuse;
    }
  }
  return r / float(SAMPLE_COUNT_DIFF);
}

vec3 IntegrateBRDF(float NdotV, float roughness) {
  float NoV = NdotV;
  float Roughness = roughness;

  vec3 res;
  vec3 V;
  V.x = sqrt(1 - NoV * NoV);
  V.y = 0;
  V.z = NoV;

  vec2 r = vec2(0.0);

  vec3 N = vec3(0.0, 0.0, 1.0);

  for (uint i = 0u; i < uint(SAMPLE_COUNT_LUT); i++) {
    vec2 E = Hammersley(i, uint(SAMPLE_COUNT_LUT));
    vec3 H = ImportanceSampleGGX(E, N, Roughness);
    vec3 L = 2 * dot(V, H) * H.xyz - V;

    float VoH = clamp(dot(V, H.xyz), 0.0, 1.0);
    float NoL = clamp(L.z, 0.0, 1.0);
    float NoH = clamp(H.z, 0.0, 1.0);

    if(NoL > 0) {
      float G = IBL_PBR_Specular_G(NoL, NoV, Roughness);
      float Gv = G * VoH / NoH;
      float Fc = (1 - VoH) * (1 - VoH);
      Fc = Fc * Fc * (1 - VoH);
      r.x = r.x + Gv;
      r.y = r.y + Gv * Fc;
    }
  }
  res.xy = r.xy / float(SAMPLE_COUNT_LUT);
  res.z = IBL_Default_DiffuseIntegrated(Roughness, NoV);
  return res;
}

void main() {
  vec3 integratedBRDF = IntegrateBRDF(v_TexCoord.x, v_TexCoord.y);
  FragColor = integratedBRDF;
}