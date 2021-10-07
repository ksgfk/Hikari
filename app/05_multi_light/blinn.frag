#version 330 core

#include <HikariLight.glsl>
#include <HikariCamera.glsl>

in vec3 v_Pos;
in vec3 v_Normal;

out vec4 f_Color;

struct BlinnPhongMaterial {
  vec3 kd;
  vec3 ks;
  float shiness;
};

uniform BlinnPhongMaterial u_blinn;

#define Kd u_blinn.kd
#define Ks u_blinn.ks
#define Shiness u_blinn.shiness

void main() {
  vec3 n = normalize(v_Normal);
  vec3 fragPos = v_Pos.xyz;

  vec3 ambient = 0.05 * Kd;

  vec3 radiance = ambient;
  for(int i = 0; i < GetDirLightCount(); i++) {
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    vec3 lightDir = GetDirLightDirection(i);
    vec3 lightRad = GetDirLightRadiance(i);

    float NdotL = dot(lightDir, n);
    diffuse = max(NdotL, 0.0) * lightRad * Kd;

    vec3 viewDir = normalize(GetCameraPos() - fragPos);
    vec3 halfDir = normalize(viewDir + lightDir);
    float spec = max(pow(dot(halfDir, n), Shiness), 0);// * step(0, NdotL);
    specular = Ks * lightRad * spec;

    radiance += diffuse + specular;
  }

  vec3 blinnColor = pow(radiance, vec3(1.0 / 2.2));
  f_Color = vec4(blinnColor, 1.0);
  //f_Color = vec4((v_Normal + 1) * 0.5, 1.0);
}