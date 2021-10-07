#ifndef HIKARI_LIGHT_INCLUDED
#define HIKARI_LIGHT_INCLUDED

#ifndef MAX_LIGHT_COUNT //默认最多8盏灯
#define MAX_LIGHT_COUNT 8
#endif

layout(std140) uniform HikariLight {
  vec3 u_LightRadianceDir[MAX_LIGHT_COUNT];
  vec3 u_LightDirectionDir[MAX_LIGHT_COUNT];
  vec3 u_LightRadiancePoint[MAX_LIGHT_COUNT];
  vec3 u_LightPositionPoint[MAX_LIGHT_COUNT];
  int u_LightDirCount;
  int u_LightPointCount;
};

vec3 GetDirLightRadiance(int idx) {
  return u_LightRadianceDir[idx];
}

vec3 GetDirLightDirection(int idx) {
  return normalize(u_LightDirectionDir[idx]);
}

int GetDirLightCount() {
  return u_LightDirCount;
}

#endif