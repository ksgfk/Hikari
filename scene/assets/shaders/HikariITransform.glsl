#ifndef HIKARI_TRANSFORM_INCLUDED
#define HIKARI_TRANSFORM_INCLUDED

uniform mat4 a_ObjectToWorld;

layout(std140) uniform HikariTransform {
  mat4 a_MatrixV;
  mat4 a_MatrixP;
  mat4 a_MatrixN;
  mat4 a_MatrixVP;
};

#endif