#version 330 core

#define PI 3.14159265359

#ifndef SAMPLE_STEP
#define SAMPLE_STEP 0.01
#endif

out vec4 f_Color;

in vec3 v_Pos;

uniform samplerCube u_Cube;

void main() {
  vec3 cubeMapTexcoord = v_Pos;

  //构建切线空间
  vec3 N = normalize(cubeMapTexcoord);
  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = cross(up, N);
  up = cross(N, right);

  float nrSamples = 0.0;
  vec3 irradiance = vec3(0.0);
  //计算整个半球上的积分，均匀采样，sinTheta作为权重
  for(float phi = 0.0; phi < 2.0 * PI; phi += SAMPLE_STEP) {
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);
    for(float theta = 0.0; theta < 0.5 * PI; theta += SAMPLE_STEP) {
      float cosTheta = cos(theta);
      float sinTheta = sin(theta);
      //球面坐标转换为直角坐标系
      vec3 tangentDir = vec3(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
      //从切线空间转换到世界空间
      vec3 worldDir = tangentDir.x * right + tangentDir.y * up + tangentDir.z * N; 
      irradiance += texture(u_Cube, worldDir).rgb * cosTheta * sinTheta;
      nrSamples++;
    }
  }
  irradiance = irradiance * (1.0 / float(nrSamples)) * PI;

  f_Color = vec4(irradiance, 1.0);
}