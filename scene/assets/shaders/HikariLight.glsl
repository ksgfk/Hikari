#ifndef HIKARI_LIGHT_INCLUDED
#define HIKARI_LIGHT_INCLUDED

#ifndef MAX_DIR_LIGHT
#define MAX_DIR_LIGHT 8
#endif

#ifndef MAX_POI_LIGHT
#define MAX_POI_LIGHT 8
#endif

layout(std140) uniform HikariLight {
  vec3 u_LightRadianceDir[MAX_DIR_LIGHT];
  vec3 u_LightDirectionDir[MAX_DIR_LIGHT];
  vec3 u_LightRadiancePoint[MAX_POI_LIGHT];
  vec3 u_LightPositionPoint[MAX_POI_LIGHT];
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

int GetPointLightCount() {
  return u_LightPointCount;
}

vec3 GetPointLightDirection(int idx, vec3 point) {
  return normalize(point - u_LightPositionPoint[idx]);
}

vec3 GetPointLightRadiance(int idx, vec3 point) {
  float distance = length(u_LightPositionPoint[idx] - point);
  return u_LightRadiancePoint[idx] / (distance * distance);
}

#endif