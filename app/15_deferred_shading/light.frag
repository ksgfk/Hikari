#version 330 core

#include <HikariLight.glsl>
#include <HikariCamera.glsl>
#include <BRDF.glsl>
#include <ToneMapping.glsl>

out vec4 f_Color;

in vec2 v_TexCoord0;

uniform sampler2D g_Pos;
uniform sampler2D g_Normal;
uniform sampler2D g_Albedo;
uniform sampler2D g_Param;

void main() {
  vec3 pos = texture(g_Pos, v_TexCoord0).xyz;
  vec3 n = normalize(texture(g_Normal, v_TexCoord0).xyz);
  vec3 v = normalize(GetCameraPos() - pos);
  float NdotV = max(dot(n, v), 0.0);
  vec3 albedo = texture(g_Albedo, v_TexCoord0).rgb;
  vec2 param = texture(g_Param, v_TexCoord0).rg;
  float metallic = param.x;
  float roughness = param.y;
  vec3 f0 = vec3(0.04);
  f0 = mix(f0, albedo, metallic);

  vec3 Lo = vec3(0.0);
  for(int i = 0; i < GetPointLightCount(); i++) {
    vec3 l = -normalize(GetPointLightDirection(i, pos));
    vec3 Li = GetPointLightRadiance(i, pos);
    vec3 fr = Cook_Torrance(v, l, n, f0, roughness, albedo, metallic);
    Lo += fr * Li;
  }

  vec3 color = ACESToneMapping(Lo, 1.0);
  color = pow(color, vec3(1.0 / 2.2));
  f_Color = vec4(color, 1.0);
}