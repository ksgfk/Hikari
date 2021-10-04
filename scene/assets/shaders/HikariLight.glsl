#ifndef HIKARI_LIGHT_INCLUDED
#define HIKARI_LIGHT_INCLUDED

#ifndef MAX_LIGHT_COUNT //默认最多8盏灯
#define MAX_LIGHT_COUNT 8
#endif

layout(std140) uniform HikariLight {
  vec3 a_LightRadiance[MAX_LIGHT_COUNT];
  vec3 a_LightDirection[MAX_LIGHT_COUNT];
};

#endif