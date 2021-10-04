#include <iostream>
#include <fstream>

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

#include <hikari/application.h>

using namespace glslang;

const char* s = R"(
#version 330 core

#include "test.glsl"
in vec3 aPos;
in vec3 aNormal;
out vec3 vPos;
out vec3 vNormal;

uniform MAT {
  mat4 projection;
  mat4 view;
  mat4 model;
};

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

const char* ext = "\n#extension GL_GOOGLE_include_directive : enable\n";

const TBuiltInResource DefaultRes = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    /* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};

TShader shader(EShLanguage::EShLangVertex);

TBuiltInResource Resource = DefaultRes;
EShMessages message = EShMsgDefault;
std::string source;
Hikari::ShaderIncluder includer{};

TProgram prog;

std::vector<uint32_t> spirv;
spv::SpvBuildLogger logger;
SpvOptions spv_opts{};

std::unique_ptr<spirv_cross::CompilerGLSL> compiler;

int main() {
  auto curr = std::filesystem::current_path();
  std::ofstream testFile(curr / "test.glsl");  //真的写个文件到硬盘里
  testFile << ts << std::endl;
  testFile.close();

  InitializeProcess();

  shader.setEnvInput(glslang::EShSourceGlsl, EShLanguage::EShLangVertex, EShClient::EShClientOpenGL, 100);
  shader.setEnvClient(EShClient::EShClientOpenGL, glslang::EshTargetClientVersion::EShTargetOpenGL_450);
  shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, EShTargetLanguageVersion::EShTargetSpv_1_0);
  shader.setAutoMapBindings(true);
  shader.setAutoMapLocations(true);
  shader.setStrings(&s, 1);
  shader.setPreamble(ext);

  {
    bool result = shader.preprocess(&Resource, 110, ENoProfile, false, false, message, &source, includer);
    std::cout << "-----------------------pre process result-----------------------" << std::endl;
    std::cout << "shader source:\n"
              << source << std::endl;
    std::cout << "result:" << result << std::endl;
    std::cout << "log:" << shader.getInfoLog() << std::endl;
    std::cout << "debug log:" << shader.getInfoDebugLog() << std::endl;
  }
  {
    //auto preamble = source.find_first_of("#extension GL_GOOGLE_include_directive : enable");
    //auto line = source.find_first_of('\n', preamble);
    //auto newSource = source.substr(line + 1);
    //auto newSource = source;

    //auto version = newSource.find("#version");
    //auto verNextLine = newSource.find('\n', version);
    //auto ver = newSource.substr(version, verNextLine - version) + "\n" + ext;
    //auto finalizeSrc = newSource.substr(verNextLine + 1);

    //auto d = ver + finalizeSrc;
    //auto finalD = d.data();
    //auto src = source.data();
    //shader.setStrings(&src, 1);
    //shader.setPreamble(ext);
    bool result = shader.parse(&Resource, 330, EProfile::ECoreProfile, true, false, EShMsgDefault, includer);
    //bool result = shader.parse(&Resource, 110, false, message, includer);
    std::cout << "-----------------------parse result-----------------------" << std::endl;
    std::cout << "result:" << result << std::endl;
    std::cout << "log:" << shader.getInfoLog() << std::endl;
    std::cout << "debug log:" << shader.getInfoDebugLog() << std::endl;
  }

  prog.addShader(&shader);
  auto lnkRes = prog.link(EShMessages::EShMsgDefault);
  std::cout << "-----------------------link result-----------------------" << std::endl;
  std::cout << "result:" << lnkRes << std::endl;
  std::cout << "log:" << prog.getInfoLog() << std::endl;
  std::cout << "debug log:" << prog.getInfoDebugLog() << std::endl;

  std::cout << "-----------------------generate SPIR-V-----------------------" << std::endl;

  spv_opts.validate = true;
  spv_opts.disableOptimizer = false;
  spv_opts.optimizeSize = true;
  spv_opts.disassemble = true;
  //auto inter = shader.getIntermediate();
  //auto exts = inter->getRequestedExtensions();
  //auto proc = inter->getProcesses();
  //for (const auto& v : exts) {
  //  std::cout << v << std::endl;
  //}
  //for (const auto& p : proc) {
  //  std::cout << p << std::endl;
  //}

  GlslangToSpv(*prog.getIntermediate(EShLanguage::EShLangVertex), spirv, &logger, &spv_opts);
  std::cout << "SPIR-V Gen Log:" << logger.getAllMessages() << std::endl;

  compiler = std::make_unique<spirv_cross::CompilerGLSL>(spirv);
  spirv_cross::ShaderResources ress = compiler->get_shader_resources();
  spirv_cross::CompilerGLSL::Options opts = compiler->get_common_options();
  opts.enable_420pack_extension = false;
  opts.emit_push_constant_as_uniform_buffer = true;
  opts.es = false;
  opts.version = 330;
  opts.flatten_multidimensional_arrays = true;
  compiler->set_common_options(opts);
  auto result = compiler->compile();
  std::cout << "code:\n"
            << result << std::endl;

  FinalizeProcess();
  return 0;
}