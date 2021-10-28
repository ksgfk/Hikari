#version 410 core

#include <BRDF.glsl>

layout (location = 0) out vec3 g_Pos;
layout (location = 1) out vec3 g_Normal;
layout (location = 2) out vec3 g_Albedo;
layout (location = 3) out vec3 g_Param;

in vec3 v_Pos;
in vec3 v_Normal;

uniform MetallicWorkflowMaterial u_pbr;

void main() {
  g_Pos = v_Pos;
  g_Normal = normalize(v_Normal);
  g_Albedo = u_pbr.Albedo;
  g_Param = vec3(u_pbr.Metallic, u_pbr.Roughness, 0);
}