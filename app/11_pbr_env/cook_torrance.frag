#version 330 core

#include <HikariLight.glsl>
#include <HikariCamera.glsl>
#include <BRDF.glsl>

in vec3 v_Pos;
in vec3 v_Normal;

out vec4 f_Color;

uniform MetallicWorkflowMaterial u_metal;
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BrdfLut;
uniform int u_MaxLod;

void main() {
  vec3 n = normalize(v_Normal);
  vec3 v = normalize(GetCameraPos() - v_Pos);
  vec3 r = reflect(-v, n); 
  float NdotV = max(dot(n, v), 0.0);
  vec3 albedo = u_metal.Albedo;
  float metallic = u_metal.Metallic;
  float roughness = u_metal.Roughness;
  vec3 f0 = vec3(0.04);
  f0 = mix(f0, albedo, metallic);

  vec3 Lo = vec3(0.0);

  //直接光计算
  for(int i = 0; i < GetDirLightCount(); i++) {
    vec3 l = -normalize(GetDirLightDirection(i));
    vec3 Li = GetDirLightRadiance(i);

    vec3 fr = Cook_Torrance(v, l, n, f0, roughness, albedo, metallic);

    Lo += fr * Li;
  }

  //间接光计算
  vec3 f = FresnelSchlickRoughness(f0, v, n, roughness);
  vec3 ks = f;
  vec3 kd = (vec3(1.0) - ks) * (1.0 - metallic);
  vec3 irradiance = texture(u_IrradianceMap, n).rgb;
  vec3 LeDiffuse = kd * irradiance * albedo;
  vec3 prefilter = textureLod(u_PrefilterMap, r, roughness * u_MaxLod).rgb;
  vec2 brdf = texture(u_BrdfLut, vec2(NdotV, roughness)).rg;
  vec3 LeBrdf = prefilter * (f * brdf.x + brdf.y);
  vec3 Le = LeDiffuse + LeBrdf;

  Lo += Le;

  vec3 color = Lo / (Lo + vec3(1.0));  //Reinhard tonemapping
  color = pow(color, vec3(1.0 / 2.2));
  f_Color = vec4(color, 1.0);
}