#ifndef HIKARI_INPUT_INCLUDED
#define HIKARI_INPUT_INCLUDED

#ifndef MAX_LIGHT_COUNT //默认最多8盏灯
#define MAX_LIGHT_COUNT 8
#endif

uniform a_ObjectToWorld;

layout(std140) uniform HikariTransform {
  mat4 a_MatrixV;
  mat4 a_MatrixP;
  mat4 a_MatrixN;
  mat4 a_MatrixVP;
};

uniform a_CameraPos;

struct __Light {//只有direction light.问题不大
  vec3 Radiance;
  vec3 Direction;
} Light;

layout(std140) uniform HikariLight {
  Light a_Light[MAX_LIGHT_COUNT];
};

#endif