#include <iostream>
#include <fstream>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

#include <hikari/application.h>

using namespace Hikari;

const char* s = R"(
#version 330 core

#include "test.glsl"
in vec3 aPos;
in vec3 aNormal;
out vec3 vPos;
out vec3 vNormal;

mat4 projection;
mat4 view;
mat4 model;

void main() {
  vPos = vec3(model * vec4(aPos, 1.0));
  vNormal = normalize(mat3(transpose(model)) * aNormal);
  gl_Position = projection * view * model * vec4(aPos, 1.0);
})";

const char* ts = R"(
uniform TEST {
  uniform vec3 a;
  uniform vec3 b;
  uniform mat4 d;
  uniform mat4 e;
};)";

int main() {
  RenderContextOpenGL ctx;
  ctx.Init("");
  auto curr = std::filesystem::current_path();
  std::ofstream testFile(curr / "test.glsl");  //真的写个文件到硬盘里
  testFile << ts << std::endl;
  testFile.close();

  std::string res;
  bool succ = ctx.ProprocessShader(ShaderType::Vertex, s, res);
  std::cout << "is success:" << succ << std::endl;
  std::cout << res << std::endl;

  return 0;
}