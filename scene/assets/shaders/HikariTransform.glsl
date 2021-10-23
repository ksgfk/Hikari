#ifndef HIKARI_TRANSFORM_INCLUDED
#define HIKARI_TRANSFORM_INCLUDED

uniform mat4 u_ObjectToWorld;
uniform mat4 u_WorldToObject;

layout(std140) uniform HikariTransform {
  mat4 u_MatrixV;
  mat4 u_MatrixInvV;
  mat4 u_MatrixP;
  mat4 u_MatrixVP;
};

mat4 Hikari_MatrixMVP() {
  return u_MatrixVP * u_ObjectToWorld;
}

vec4 HikariObjectToClipPos(vec3 pos) {
  return Hikari_MatrixMVP() * vec4(pos, 1.0);
}

vec3 HikariObjectToWorldPos(vec3 pos) {
  vec4 homo = u_ObjectToWorld * vec4(pos, 1.0);
  return homo.xyz / homo.w;
}

vec3 HikariWorldNormal(vec3 normal) {
  return normalize(transpose(mat3(u_WorldToObject)) * normal);
}

vec3 HikariViewNormal(vec3 normal) {
  return normalize(transpose(mat3(u_WorldToObject * u_MatrixInvV)) * normal);
}

vec3 BiTangent(vec3 normal, vec4 tangent) {
  return cross(normal, tangent.xyz) * tangent.w;
}

mat3 TBN(vec3 normal, vec4 tangent) {
  vec3 T = tangent.xyz;
  vec3 B = BiTangent(normal, tangent);
  vec3 N = normal;
  
  return mat3(T, B, N);
}

vec3 HikariTangentNormal(vec3 normal, mat3 tbn) {
  return normalize(tbn * normal);
}

vec3 SampleNormalMap(sampler2D map, vec2 texCoord, mat3 tbn) {
  vec3 N = texture(map, texCoord).rgb;
  N = normalize(N * 2.0 - 1.0);   
  N = HikariTangentNormal(N, tbn);
  return N;
}

#endif