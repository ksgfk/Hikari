#version 330 core

#include <HikariLight.glsl>
#include <HikariCamera.glsl>
#include <BRDF.glsl>

in vec3 v_Pos;
in vec3 v_Normal;

out vec4 f_Color;

uniform MetallicWorkflowMaterial u_metal;
uniform samplerCube u_irradianceMap;

void main() {
  vec3 n = normalize(v_Normal);
  vec3 v = normalize(GetCameraPos() - v_Pos);
  float NdotV = max(dot(n, v), 0.0);
  vec3 albedo = u_metal.Albedo;
  float metallic = u_metal.Metallic;
  float roughness = u_metal.Roughness;
  vec3 f0 = vec3(0.04);
  f0 = mix(f0, albedo, metallic);

  vec3 Lo = vec3(0.0);
  for(int i = 0; i < GetDirLightCount(); i++) {
    vec3 l = -normalize(GetDirLightDirection(i));
    vec3 h = normalize(v + l);
    float NdotL = max(dot(n, l), 0.0); 

    vec3 Li = GetDirLightRadiance(i);

    float NDF = DistributionGGX(n, h, roughness);   
    float G = GeometrySmith(n, v, l, roughness); 
    vec3 F = FresnelSchlick(f0, v, h);
    vec3 numerator = NDF * G * F; 
    float denominator = max((4.0 * NdotL * NdotV), 0.001);
    vec3 cookTorrance = numerator / denominator;

    vec3 ks = FresnelSchlickRoughness(f0, v, h, roughness);
    vec3 kd = (vec3(1.0) - ks) * (1.0 - metallic);
    vec3 irradiance = texture(u_irradianceMap, n).rgb;
    vec3 diffuse = kd * albedo * irradiance;

    vec3 brdf = cookTorrance + diffuse;

    Lo += brdf * Li * NdotL;
  }
  vec3 color = Lo / (Lo + vec3(1.0));  //Reinhard tonemapping
  color = pow(color, vec3(1.0 / 2.2));
  f_Color = vec4(color, 1.0);
}