#version 330 core

#include <HikariLight.glsl>
#include <HikariCamera.glsl>
#include <BRDF.glsl>

in vec3 v_Pos;
in vec3 v_Normal;

out vec4 f_Color;

uniform BlinnPhongMaterial u_blinn;

void main() {
  vec3 n = normalize(v_Normal);
  vec3 fragPos = v_Pos.xyz;
  vec3 cameraPos = GetCameraPos();

  vec3 ambient = 0.05 * u_blinn.kd;

  vec3 radiance = ambient;
  for(int i = 0; i < GetDirLightCount(); i++) {
    radiance += BlinnPhong(u_blinn, n, -GetDirLightDirection(i), GetDirLightRadiance(i), cameraPos, fragPos);
  }

  vec3 blinnColor = pow(radiance, vec3(1.0 / 2.2));
  f_Color = vec4(blinnColor, 1.0);
}