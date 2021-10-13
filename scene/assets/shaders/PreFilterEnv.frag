#version 330 core

#ifndef SAMPLE_COUNT
#define SAMPLE_COUNT 2048
#endif

#define PI 3.14159265359

out vec4 f_Color;

in vec3 v_Pos;

uniform float u_roughness;
uniform samplerCube u_Cube;

// 低差异序列
vec2 Hammersley(uint i, uint N) {
  uint bits = i;
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  float rdi = float(bits) * 2.3283064365386963e-10; // / 0x100000000
  return vec2(float(i) / float(N), rdi);
}

// GGX重要性采样
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
  float a = roughness * roughness;
  // 球面坐标
  //float thetaM = atan(a * sqrt(Xi.x) / sqrt(1 - Xi.x));
  //float phiH = 2 * PI * Xi.y;
  float phi = 2.0 * PI * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
  // 转换到直角坐标系
  //vec3 wi = vec3(sin(thetaM) * cos(phiH), sin(thetaM) * sin(phiH), cos(thetaM));
  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;
  // 构造切线空间
  //float invLen = 1.0f / sqrt(N.x * N.x + N.z * N.z);
  //vec3 T = vec3(N.z * invLen, 0.0f, -N.x * invLen);
  //vec3 B = cross(T, N);
  vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);
  // 转换到世界空间
  //vec3 dir = vec3(dot(B, wi), dot(T, wi), dot(N, wi));
  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
  //return normalize(dir);
  return normalize(sampleVec);
}

void main() {
  vec3 N = normalize(v_Pos);
  // 假设反射方向总是等于采样方向
  vec3 R = N;
  vec3 V = R;

  float weight = 0.0;
  vec3 color = vec3(0.0);
  for(uint i = 0u; i < uint(SAMPLE_COUNT); i++) {
    vec2 Xi = Hammersley(i, uint(SAMPLE_COUNT));
    vec3 H = ImportanceSampleGGX(Xi, N, u_roughness);
    vec3 L = normalize(2.0 * dot(V, H) * H - V);
    float NdotL = max(dot(N, L), 0.0);
    color += texture(u_Cube, L).rgb * NdotL * step(0, NdotL);
    weight += NdotL * step(0, NdotL);
  }
  color /= weight;

  f_Color = vec4(color, 1.0);
}