#version 330 core

#include <HikariTransform.glsl>
#include <HikariLight.glsl>
#include <HikariCamera.glsl>
#include <BRDF.glsl>
#include <ToneMapping.glsl>

in vec3 v_Pos;
in vec2 v_TexCoord;
in mat3 v_TBN;

out vec4 f_Color;

uniform sampler2D u_Albedo;
uniform sampler2D u_Roughness;
uniform sampler2D u_NormalMap;
uniform float u_Metallic;

void main() {
  vec3 n = SampleNormalMap(u_NormalMap, v_TexCoord, v_TBN);
  vec3 v = normalize(GetCameraPos() - v_Pos);
  float NdotV = max(dot(n, v), 0.0);
  vec3 albedo = texture(u_Albedo, v_TexCoord).rgb;
  float metallic = u_Metallic;
  float roughness = texture(u_Roughness, v_TexCoord).r;
  vec3 f0 = vec3(0.04);
  f0 = mix(f0, albedo, metallic);

  vec3 Lo = vec3(0.0);
  for(int i = 0; i < GetDirLightCount(); i++) {
    vec3 l = -normalize(GetDirLightDirection(i));
    vec3 Li = GetDirLightRadiance(i);
    vec3 fr = Cook_Torrance(v, l, n, f0, roughness, albedo, metallic);
    Lo += fr * Li;
  }

  vec3 color = ACESToneMapping(Lo, 1.0);
  color = pow(color, vec3(1.0 / 2.2));
  f_Color = vec4(color, 1.0);
}