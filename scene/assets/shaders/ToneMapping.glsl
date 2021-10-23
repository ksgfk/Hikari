#ifndef HIKARI_TONEMAPPING_INCLUDED
#define HIKARI_TONEMAPPING_INCLUDED

//https://zhuanlan.zhihu.com/p/21983679
vec3 ReinhardToneMapping(vec3 color, float adapted_lum) {
  const float MIDDLE_GREY = 1;
  color *= MIDDLE_GREY / adapted_lum;
  return color / (1.0f + color);
}

vec3 ACESToneMapping(vec3 color, float adapted_lum) {
  const float A = 2.51f;
  const float B = 0.03f;
  const float C = 2.43f;
  const float D = 0.59f;
  const float E = 0.14f;
  color *= adapted_lum;
  return (color * (A * color + B)) / (color * (C * color + D) + E);
}

#endif